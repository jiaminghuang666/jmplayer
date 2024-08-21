//
// Created by jiaming.huang on 2024/4/2.
//

#ifndef JMPLAYER_JMPLAYER_H
#define JMPLAYER_JMPLAYER_H

#include <string>
#include <jni.h>

#include "JMDemux.h"
#include "JMDecode.h"
#include "JMAudioPlay.h"
#include "JMResample.h"
#include "XThread.h"
#include "JMVideoView.h"



#include <thread>
#include <mutex>
#include "XMsgQueue.h"

class JMPlayer :public XThread {
public:
    static JMPlayer *Get(unsigned char index = 0);
    int InitHard(void *vm);
    int PlayerBuilder(int (*msg_loop)(void*),JMPlayer *player);
    virtual int Open(const char *myurl);
    virtual void Close();
    virtual bool Start();
    virtual void Pause(bool isPause);
    virtual int InitView(void * win);

    virtual double getCurrentPosition();
    virtual int getDuration();
    virtual bool Seek(double Position);

    virtual  int getVideoHeight();
    virtual  int getVideoWidth();

    virtual int getMsg(AVMessage *msg);
    virtual int setMsg(int what, int arg1,int arg2);
    //virtual int jmplayer_msg_loop(void *arg);
    int (*msg_loop)(void *);

private:
    JMDemux *myDemux = NULL;
    JMDecode *vdecode = NULL;
    JMDecode *adecode = NULL;
    JMAudioPlay *audioPlay = NULL;
    XParameter outPara;
    JMResample *resample = NULL;
    JMVideoView *view = NULL;


    AVMessage *msg;
    Node *node = NULL;

protected:
    void Main();
    std::mutex mux;

};

#endif //JMPLAYER_JMPLAYER_H
