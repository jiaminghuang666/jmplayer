//
// Created by jiaming.huang on 2024/4/27.
//

#include "SLAudioPlay.h"
#include "JMLog.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static SLObjectItf engineSL = NULL;
static SLEngineItf eng = NULL;
static SLObjectItf mix = NULL;
static SLObjectItf player = NULL;
static SLPlayItf iplayer = NULL;
static SLAndroidSimpleBufferQueueItf pcmQue = NULL;

SLAudioPlay::SLAudioPlay()
{
   buf = new unsigned char[1024*1024];
}

SLAudioPlay::~SLAudioPlay()
{
    delete buf;
    buf = 0;
}

static SLEngineItf CreateSL()
{
    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL,0,0,0,0,0);
    if(re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->Realize(engineSL,SL_BOOLEAN_FALSE);
    if(re != SL_RESULT_SUCCESS) return NULL;
    re = (*engineSL)->GetInterface(engineSL,SL_IID_ENGINE,&en);
    if(re != SL_RESULT_SUCCESS) return NULL;

    ALOGD("SLEngineItf CreateSL ==success== ");
    return en;
}


void SLAudioPlay::PlayCall(void *bufq)
{
    if (!bufq) {
        ALOGE("SLAudioPlay::PlayCall bufq == NULL \n");
        return;
    }
    SLAndroidSimpleBufferQueueItf bf = (SLAndroidSimpleBufferQueueItf)bufq;

    XData d = GetData();
    if (d.size <= 0) {
        ALOGE("SLAudioPlay::PlayCall GetData size is 0 \n");
        return ;
    }
    if (!buf) {
        ALOGE("SLAudioPlay::PlayCall buf == NULL \n");
        return ;
    }

    memcpy(buf, d.data, d.size);
    mux.lock();
    if (pcmQue && (*pcmQue))
         (*pcmQue)->Enqueue(pcmQue,buf,d.size);
    mux.unlock();
    d.Drop();
    //ALOGD("SLAudioPlay::PlayCall ==success== ");
}

void PcmCall(SLAndroidSimpleBufferQueueItf bf,void *contex)
{
    //ALOGD("PcmCall");
    SLAudioPlay *ap = (SLAudioPlay *)contex;
    if (!ap){
        ALOGE("SLAudioPlay ap == NULL failed!");
        return;
    }
    ap->PlayCall((void *)bf);
}

void SLAudioPlay::Close()
{
    JMAudioPlay::Clear();

    mux.lock();
    if (iplayer && (*iplayer)) {  //停止播放
        (*iplayer)->SetPlayState(iplayer,SL_PLAYSTATE_STOPPED);
    }
    if (pcmQue &&(*pcmQue)) {  //清除队列
        (*pcmQue)->Clear(pcmQue);
    }
    if (player && (*player)) { //销毁player对象
        (*player)->Destroy(player);
    }
    if (mix && (*mix)) {  //销毁混音器
        (*mix)->Destroy(mix);
    }
    if (engineSL && (*engineSL)) { //销毁播放引擎
        (*engineSL)->Destroy(engineSL);
    }

    engineSL = NULL;
    eng = NULL;
    mix = NULL;
    player = NULL;
    iplayer = NULL;
    pcmQue = NULL;

    mux.unlock();
}

bool SLAudioPlay::StartPlay(XParameter out)
{
    Close();
    mux.lock();
    //1 创建引擎
    eng = CreateSL();
    if(!eng){
        mux.unlock();
        ALOGE("CreateSL fail !！ ");
        return false;
    }
    ALOGD("SLAudioPlay::StartPlay CreateSL ==success==");

    //2 创建混音器
    SLresult re = 0;
    re = (*eng)->CreateOutputMix(eng,&mix,0,0,0);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("SL_RESULT_SUCCESS failed!");
        return false;
    }
    re = (*mix)->Realize(mix,SL_BOOLEAN_FALSE);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("(*mix)->Realize failed!");
        return false;
    }
    SLDataLocator_OutputMix outmix = {SL_DATALOCATOR_OUTPUTMIX,mix};
    SLDataSink audioSink= {&outmix,0};
    ALOGD("SLAudioPlay::StartPlay CreateOutputMix ==success==");

    //3 配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,10};
    //音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            (SLuint32)out.channels,//    声道数
            (SLuint32)out.sample_rate *1000,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN //字节序，小端
    };
    SLDataSource ds = {&que,&pcm};
    ALOGD("SLAudioPlay::StartPlay configure ==success==");

    //4 创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng,&player,&ds,&audioSink,sizeof(ids)/sizeof(SLInterfaceID),ids,req);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("CreateAudioPlayer failed!");
        return false;
    }
    (*player)->Realize(player,SL_BOOLEAN_FALSE);
    //获取player接口
    re = (*player)->GetInterface(player,SL_IID_PLAY,&iplayer);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("GetInterface SL_IID_PLAY failed!");
        return false;
    }
    re = (*player)->GetInterface(player,SL_IID_BUFFERQUEUE,&pcmQue);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("GetInterface SL_IID_BUFFERQUEUE failed!");
        return false;
    }
    ALOGD("SLAudioPlay::StartPlay GetInterface ==success==");

    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue,PcmCall,this);

    //设置为播放状态
    (*iplayer)->SetPlayState(iplayer,SL_PLAYSTATE_PLAYING);

    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue,"",1);

    isExit = false;

    mux.unlock();
    ALOGD("SLAudioPlay::StartPlay ==success==");

    return true;
}