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
#include "GLVideoView.h"

#include "FFResample.h"
#include "SLAudioPlay.h"



extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
/*
std::string test(int a ,int b)
{
    std::string getVersion = "ffmpeg Version";
    getVersion += avcodec_configuration();
    ALOGD("jiaming test %s \n", getVersion.c_str());

    return getVersion.c_str();
}*/

int JMPlayer::InitHard(void *vm)
{
    ALOGD("Jmplayer  JniLoader");
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
    ALOGD("JMPlayer::getMsg ");
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
    ALOGD("Jmplayer  jmplayer_msg_loop enter");
    JMPlayer *myJMPlayer = (JMPlayer *)arg;

    myJMPlayer->msg_loop(arg);

    ALOGD("Jmplayer  jmplayer_msg_loop exit ");

    return 0;
}

int JMPlayer::PlayerBuilder(int (*msg_loop)(void*), JMPlayer *player)
{
    int ret = 0;

    ALOGD("Jmplayer  PlayerBuilder enter");

    player->msg_loop = msg_loop;
    std::thread JmPlayerThread(jmplayer_msg_loop, player);
    JmPlayerThread.detach();

    ALOGD("Jmplayer  PlayerBuilder enter 1");

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

    ALOGD("Jmplayer  PlayerBuilder exit");

    ALOGD("Jmplayer  PlayerBuilder exit 2");
    return ret;
}


void JMPlayer::Main()
{
    while (!isExit) {
        mux.lock();
        if(!audioPlay || !vdecode) {
            ALOGD("JMPlayer::Main apts 2");
            mux.unlock();
            XSleep(2);
            continue;
        }

        //同步
        //获取音频的pts 告诉视频，控制视频的解码时间
        int apts = audioPlay->pts;
        //ALOGD("JMPlayer::Main apts = %d", apts);
        vdecode->synPts = apts;

        mux.unlock();
        XSleep(2);
    }
}

void JMPlayer::Close()
{
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
}

int JMPlayer::Open(const char *myurl)
{
    Close();

    mux.lock();
    int ret = 0;
    if (!myDemux || !myDemux->Open(myurl)) {
        mux.unlock();
        ALOGE("JMPlayer::Open demux fail !!");
        return false;
    }
    // create video deocder
    if (!vdecode || !vdecode->CreateDecode( myDemux->getVPara(), true)) {
        ALOGE("JMPlayer::Open video CreateDecode fail !!");
        //return false;
    }

    // create audio deocder
    if (!adecode || !adecode->CreateDecode( myDemux->getAPara(), false)) {
        ALOGE("JMPlayer::Open audio CreateDecode fail !!");
        //return false;
    }

    // create video resample
    //if (outPara.sample_rate <= 0)
        outPara = myDemux->getAPara();
    if ( !resample || !resample->Open(myDemux->getAPara(),outPara)) {
        ALOGE("JMPlayer::Open audio CreateDecode fail !!");
        //return false;
    }

    ALOGD("JMPlayer::Open  success %s", myurl);
    mux.unlock();
    return ret;
}

bool JMPlayer::Start()
{
    mux.lock();
    if (!vdecode || !vdecode->StartThread()) {
        ALOGE("JMPlayer::Start video decoder fail ");
        //return false;
    }

    if (!myDemux || !myDemux->StartThread()) {
        //mux.unlock();
        ALOGE("JMPlayer::Start demux fail ");
        return false;
    }

    if(!adecode || !adecode->StartThread()) {
        ALOGE("JMPlayer::Start audio decoder fail ");
        //return false;
    }

    if (!audioPlay || !audioPlay->StartPlay(outPara)){
        ALOGE("JMPlayer::Start audioPlay StartPlay fail ");
        //return false;
    }
    ALOGD("JMPlayer::Start audioPlay outPara sample_rate=%d channels=%d ",outPara.sample_rate,outPara.channels );

    XThread::StartThread();

    mux.unlock();
    return true;
}

void JMPlayer::Pause(bool isPause)
{
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

int JMPlayer::InitView(void * win)
{
    if (view ) {
        view->Close();
        view->SetRender(win);
    }
    return 0;
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
            //ALOGD("JMPlayer::GetPosition  pts =%d duration=%d position=%f", vdecode->pts,duration,position);
        }
    }

    mux.unlock();
    return position;
}

int JMPlayer::getDuration()
{
    int duration = 0;
    mux.lock();
    if (myDemux)
        duration = myDemux->durationMs;

    mux.unlock();
    return duration;
}

bool JMPlayer::Seek(double Position)
{
    ALOGD("seek start ");
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
    ALOGD("seek end ");
    return ret;
}

int JMPlayer::getVideoHeight()
{
    int height = 0;

    mux.lock();
    if (vdecode) {
        height =  vdecode->outHeight;
    }
    mux.unlock();

    return height;
}

int JMPlayer::getVideoWidth()
{
    int Width = 0;

    mux.lock();
    if (vdecode) {
        Width =  vdecode->outWidth;
    }
    mux.unlock();

    return Width;
}