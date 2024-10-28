//
// Created by jiaming.huang on 2024/4/27.
//

#include "JMAudioPlay.h"
#include "JMLog.h"
#
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

XData  JMAudioPlay::DequeuePCM()
{
    XData d;

    isRuning = true;

    while (!isExit) {
        if (IsPause()) {
            XSleep(2);
            continue;
        }

        framesMutex.lock();
        if(!frames.empty()) {
            d = frames.front();
            frames.pop_front();
            framesMutex.unlock();
            //apts = d.pts;
            return d;
        }
        framesMutex.unlock();
        XSleep(1);
    }

    isRuning = false;

    return d;
}

void JMAudioPlay::Clear()
{
    framesMutex.lock();
    while (!frames.empty()){
        frames.front().Drop();
        frames.pop_front();
    }
    framesMutex.unlock();
}


void JMAudioPlay::EnqueuePCM(XData data)
{
    if(data.size <= 0 || !data.data) {
        ALOGE("JMAudioPlay::Update data == MULL fail \n");
        return ;
    }

    while (!isExit) {  //把数据压入队列中
        framesMutex.lock();

        if (frames.size() > maxFrame) {
            framesMutex.unlock();
            XSleep(1);
            continue;
        }
        frames.push_back(data);
        framesMutex.unlock();
        break;
    }
}


void JMAudioPlay::Update(XData data)
{
    //ALOGD("JMAudioPlay::Update \n");
    EnqueuePCM(data);
}


//==================SLAudioPlay==========================
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
        ALOGE("[%s:%d]bufq == NULL \n",__func__, __LINE__);
        return;
    }
    SLAndroidSimpleBufferQueueItf bf = (SLAndroidSimpleBufferQueueItf)bufq;

    XData d = DequeuePCM();
    if (d.size <= 0) {
        ALOGE("[%s:%d] GetData size is 0 \n",__func__, __LINE__);
        return ;
    }
    if (!buf) {
        ALOGE("[%s:%d] buf == NULL \n",__func__, __LINE__);
        return ;
    }

    apts = d.pts;  //audio clk
    ALOGE("[%s:%d] Render audio: isAudio=%d audioFramecount=%ld apts=%ld \n",__func__, __LINE__, d.isAudio, d.audioFramecount,d.pts);

    memcpy(buf, d.data, d.size);
    mux.lock();
    if (pcmQue && (*pcmQue))
        (*pcmQue)->Enqueue(pcmQue,buf,d.size);

    mux.unlock();
    d.Drop();
}

void PcmCall(SLAndroidSimpleBufferQueueItf bf,void *contex)
{
    //ALOGD("PcmCall");
    SLAudioPlay *ap = (SLAudioPlay *)contex;
    if (!ap){
        ALOGE("[%s:%d]  ap == NULL failed!",__func__, __LINE__);
        return;
    }
    ap->PlayCall((void *)bf);
}

void SLAudioPlay::Close()
{
    ALOGD("[%s:%d] success !!",__func__, __LINE__);
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


bool SLAudioPlay::initAudioPlay(XParameter out)
{
    Close();
    mux.lock();
    //1 创建引擎
    eng = CreateSL();
    if(!eng){
        mux.unlock();
        ALOGE("[%s:%d] CreateSL failed!",__func__, __LINE__);
        return false;
    }

    ALOGD("[%s:%d]  CreateSL ==success==",__func__, __LINE__);
    //2 创建混音器
    SLresult re = 0;
    re = (*eng)->CreateOutputMix(eng,&mix,0,0,0);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("[%s:%d] failed!",__func__, __LINE__);
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
    ALOGD("[%s:%d]  CreateSL mix ==success==",__func__, __LINE__);

    //3 配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,10};
    //音频格式
    ALOGD("[%s:%d]  configure param: out.channels=%d out.sample_rate *1000=%d ",__func__, __LINE__,out.channels, out.sample_rate *1000);
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
    ALOGD("[%s:%d]  CreateSL ds ==success==",__func__, __LINE__);

    //4 创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng,&player,&ds,&audioSink,sizeof(ids)/sizeof(SLInterfaceID),ids,req);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("[%s:%d] CreateAudioPlayer failed!",__func__, __LINE__);
        return false;
    }
    (*player)->Realize(player,SL_BOOLEAN_FALSE);
    //获取player接口
    re = (*player)->GetInterface(player,SL_IID_PLAY,&iplayer);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("[%s:%d] GetInterface failed!",__func__, __LINE__);
        return false;
    }
    re = (*player)->GetInterface(player,SL_IID_BUFFERQUEUE,&pcmQue);
    if(re !=SL_RESULT_SUCCESS )
    {
        mux.unlock();
        ALOGE("[%s:%d] GetInterface failed!",__func__, __LINE__);
        return false;
    }
    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue,PcmCall,this);

    //设置为播放状态
    (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PAUSED);

    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue,"",1);

    isExit = false;

    mux.unlock();
    ALOGD("[%s:%d] StartPlay ==success==!",__func__, __LINE__);


    return true;
}


bool SLAudioPlay::StartPlay()
{
    if (iplayer && (*iplayer)) {  //停止播放
        ALOGD("[%s:%d]  ==success==!",__func__, __LINE__);
        (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PLAYING);
        return true;
    } else {
        ALOGD("[%s:%d]  fail!!!",__func__, __LINE__);
        return false;
    }
}
//==================SLAudioPlay==========================