// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("JMPlayer");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("JMPlayer")
//      }
//    }


#include "JMPlayer.h"
//#include <android/log.h>
#include "JMLog.h"

#include "FFDemux.h"
#include "FFDecode.h"

#include "XThread.h"
#include "JMObserver.h"

#include "JMVideoView.h"

#include "FFResample.h"
#include "JMAudioPlay.h"

#include "utils.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int JMPlayer::InitHard(void *vm)
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    FFDecode::InitHard(vm);
    return 0;
}

JMPlayer *JMPlayer::Get(unsigned char index)
{
    static JMPlayer p[256];
    return &p[index];
}

int JMPlayer::getMsg(AVMessage *msg)
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    mux.lock();
    msg_queue_get(&node, msg);
    mux.unlock();
    return 0;
}


int JMPlayer::setMsg(int what, int arg1,int arg2)
{
    mux.lock();
    msg_queue_put_simple(&node,  what, arg1, arg2);
    mux.unlock();

    return 0;
}

int jmplayer_msg_loop(void *arg)
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    JMPlayer *myJMPlayer = (JMPlayer *)arg;

    myJMPlayer->msg_loop(arg);


    return 0;
}

int JMPlayer::PlayerBuilder(int (*msg_loop)(void*), JMPlayer *player)
{
    int ret = 0;

    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);

    player->msg_loop = msg_loop;
    std::thread JmPlayerThread(jmplayer_msg_loop, player);
    JmPlayerThread.detach();

    myDemux = new FFDemux();  //解封装
    vdecode = new FFDecode(); //视频解码
    adecode = new FFDecode(); //音频解码

    myDemux->AddObs(vdecode);  //解码器观察解封装
    myDemux->AddObs(adecode);

    view = new GLVideoView();   //显示观察视频解码器
    vdecode->AddObs(view);

    resample = new FFResample();  //重采样观察音频解码器
    adecode->AddObs(resample);

    audioPlay = new SLAudioPlay();  //音频播放观察重采样
    resample->AddObs(audioPlay);


    return ret;
}

int JMPlayer::InitView(void * win)
{
    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    if (view ) {
        view->Close();
        view->SetRender(win);
    }
    return 0;
}

void JMPlayer::VideoDisplay(double * remainTime)
{
    *remainTime = 2.0;
    //同步
    //获取音频的pts 告诉视频，控制视频的解码时间

    return;
}

void JMPlayer::Main()  //render
{
   double remainTime = 0.0;

    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    if(!view || !audioPlay ) {
        ALOGE("[%s:%d] audioPlay=NULL vdecode=NULL ",__func__, __LINE__);
        XSleep(2);
    }

    XData xdata = view->DequeueSurface();
    if (!view->initSurface(&xdata)) {//init surface
        ALOGE("[%s:%d] audioPlay=NULL vdecode=NULL ",__func__, __LINE__);
        return ;
    }
    if (!audioPlay || !audioPlay->StartPlay()) {      // render audio
        ALOGE("[%s:%d] audioPlay=NULL ",__func__, __LINE__);
        //return false;
    }
    int firstapts = audioPlay->apts;
    int firstvpts = view->vpts;
    ALOGD("VideoDisplay ++++first++ apts = %ld vpts = %ld", firstapts, firstvpts);

    while (!isExit) {
        mux.lock();
        if(!audioPlay || !vdecode ) {
            ALOGE("[%s:%d] audioPlay=NULL vdecode=NULL ",__func__, __LINE__);
            mux.unlock();
            XSleep(2);
            continue;
        }

        //if(remainTime > 0.0)
        //    XSleep(remainTime);
        //VideoDisplay(&remainTime);  //render video

        int apts = audioPlay->apts;
        int vpts = 0;

        long long t1 = GetNowMs();
        XData xdata = view->DequeueSurface();
        vpts = view->vpts;
        ALOGD("VideoDisplay apts = %ld vpts = %ld", apts, vpts);

        view->Render(&xdata);
        long long t2 = GetNowMs();
        long long diff = t2 - t1;

        ALOGD("VideoDisplay t1 = %lld t2 = %lld diff=%lld", t1, t2,diff);

        mux.unlock();
    }

    return ;
}

void JMPlayer::Close()
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    mux.lock();

    XThread::StopThread(); //停止同步线程

    if (myDemux)  //停止demux
        myDemux->StopThread();

    if (vdecode) //停止video decode
        vdecode->StopThread();
    if (adecode) //停止audio decode
        adecode->StopThread();
    if (audioPlay)
        audioPlay->StopThread();

    //2 清理缓冲队列
    if (vdecode)
        vdecode->Clear();
    if (adecode)
        adecode->Clear();
    if (audioPlay)
        audioPlay->Clear();
    if (view)
        view->Clear();

    //3 清理资源
    if (audioPlay)
        audioPlay->Close();
    if (view)
        view->Close();
    if (vdecode)
        vdecode->stopDecode();
    if (adecode)
        adecode->stopDecode();
    if (myDemux)
        myDemux->Close();

    mux.unlock();
    ALOGD("[%s:%d] ==end==",__func__, __LINE__);
}

int JMPlayer::setDataSource(const char *myurl)
{
    Close();

    ALOGD("[%s:%d] Open %s ==enter==",__func__, __LINE__,myurl);
    mux.lock();
    int ret = 0;
    if (!myDemux || !myDemux->Open(myurl)) {
        mux.unlock();
        ALOGE("[%s:%d] myDemux=null fail!!",__func__, __LINE__);
        return false;
    }
    // create video deocder
    if (!vdecode || !vdecode->CreateDecode( myDemux->getVPara(), false)) {
        ALOGE("[%s:%d] vdecode=null fail!!",__func__, __LINE__);
        //return false;
    }

    // create audio deocder
    if (!adecode || !adecode->CreateDecode( myDemux->getAPara(), false)) {
        ALOGE("[%s:%d] adecode=null fail!!",__func__, __LINE__);
        //return false;
    }

    // create video resample
    //if (outPara.sample_rate <= 0)
    outPara = myDemux->getAPara();
    if ( !resample || !resample->Open(myDemux->getAPara(),outPara)) {
        ALOGE("[%s:%d] resample=null fail!!",__func__, __LINE__);
        //return false;
    }

    ALOGD("JMPlayer::Start audioPlay outPara sample_rate=%d channels=%d ",outPara.sample_rate,outPara.channels );
    if (!audioPlay || !audioPlay->initAudioPlay(outPara)){
        ALOGE("[%s:%d] audioPlay=null fail!!",__func__, __LINE__);
        return false;
    }

    mux.unlock();
    ALOGD("[%s:%d] ==end==",__func__, __LINE__);
    return ret;
}

int JMPlayer::prepareAsync()
{

    return 0;
}

bool JMPlayer::Start()
{
    mux.lock();
    ALOGD("[%s:%d]  myDemux->StartThread ==start==",__func__, __LINE__);
    if (!myDemux || !myDemux->StartThread()) {
        //mux.unlock();
        ALOGE("[%s:%d] myDemux=NULL ",__func__, __LINE__);
        return false;
    }

    ALOGD("[%s:%d]  vdecode->StartThread ==start==",__func__, __LINE__);
    if (!vdecode || !vdecode->StartThread()) {
        ALOGE("[%s:%d] vdecode=NULL ",__func__, __LINE__);
        //return false;
    }

    ALOGD("[%s:%d]  adecode->StartThread ==start==",__func__, __LINE__);
    if(!adecode || !adecode->StartThread()) {
        ALOGE("[%s:%d] adecode=NULL ",__func__, __LINE__);
        //return false;
    }

    XThread::StartThread();   // start render

    mux.unlock();
    ALOGD("[%s:%d] ==end==",__func__, __LINE__);
    return true;
}

void JMPlayer::Pause(bool isPause)
{
    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    mux.lock();
    XThread::SetPause(isPause);
    if (myDemux)
        myDemux->SetPause(isPause);
    if (vdecode)
        vdecode->SetPause(isPause);
    if (adecode)
        adecode->SetPause(isPause);
    if (audioPlay)
        audioPlay->SetPause(isPause);
    mux.unlock();
}



double JMPlayer::getCurrentPosition()
{
    double position = 0.0;
    int duration = 0;

    mux.lock();
    if (myDemux)
        duration = myDemux->durationMs;
    if (duration > 0) {
        if (vdecode) {
            position = (double )vdecode->pts / (double )duration;
            //ALOGD("[%s:%d]  pts =%d duration=%d position=%f",__func__, __LINE__, vdecode->pts,duration,position);
        }
    }

    mux.unlock();
    return position;
}

int JMPlayer::getDuration()
{
    int duration = 0;
    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    mux.lock();
    if (myDemux)
        duration = myDemux->durationMs;

    mux.unlock();
    return duration;
}

bool JMPlayer::Seek(double Position)
{
    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    bool ret = false;
    if (!myDemux) return false;

    Pause(true); //1 先暂停
    mux.lock();  //2 清理缓冲队列
    if (vdecode)
        vdecode->Clear();
    if (adecode)
        adecode->Clear();
    if (audioPlay)
        audioPlay->Clear();

    ret = myDemux->Seekto(Position);  //3 跳到关键帧

    if (!vdecode) {
        mux.unlock();
        Pause(false);
        return ret;
    }

    // 4 解码 seek对应位置的数据
    int Seekpts = Position * myDemux->durationMs;
    while (!isExit) {

        XData  pkt = myDemux->Read();
        if (pkt.size < 0 ) break;
        if (pkt.isAudio) {
            if (pkt.pts < Seekpts) {
                pkt.Drop();
                continue;
            }
            myDemux->Notify(pkt);  // 4.1 写入缓冲队列
            continue;
        }

        //4.2 解码数据发给显示队列中
        vdecode->sendPacket(pkt);
        pkt.Drop();
        XData data = vdecode->receiveFrame();
        if (data.size <= 0 ) {
            continue;
        }
        if (data.pts >= Seekpts) {
            break;
        }
    }

    Pause(false);

    mux.unlock();
    ALOGD("[%s:%d]  ==end==",__func__, __LINE__);
    return ret;
}

int JMPlayer::getVideoHeight()
{
    int height = 0;

    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    mux.lock();
    if (vdecode) {
        height =  vdecode->outHeight;
    }
    mux.unlock();
    ALOGD("[%s:%d]  height=%d ",__func__, __LINE__,height);
    return height;
}

int JMPlayer::getVideoWidth()
{
    int Width = 0;

    ALOGD("[%s:%d]  ==enter==",__func__, __LINE__);
    mux.lock();
    if (vdecode) {
        Width =  vdecode->outWidth;
    }
    mux.unlock();

    ALOGD("[%s:%d]  height=%d ",__func__, __LINE__,Width);
    return Width;
}