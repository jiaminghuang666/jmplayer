//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_JMAUDIOPLAY_H
#define JMPLAYER_JMAUDIOPLAY_H

#include "JMObserver.h"
#include "XParameter.h"

#include <mutex>
#include <list>

class JMAudioPlay:public JMObserver {
public:
    virtual void Clear();
    virtual XData DequeuePCM();
    virtual void EnqueuePCM(XData data);
    virtual void Update(XData data);


    virtual bool initAudioPlay(XParameter out) = 0;
    virtual bool  StartPlay() = 0;
    virtual void Close() = 0;
    int maxFrame = 50;
    int apts = 0;
protected:
    std::mutex framesMutex;
    std::list <XData> frames;
};

class SLAudioPlay :public JMAudioPlay {
public:
    virtual bool initAudioPlay(XParameter out);
    virtual bool StartPlay();
    virtual void Close();
    void PlayCall(void *bufq);

    SLAudioPlay();
    virtual ~SLAudioPlay();

protected:
    unsigned  char *buf = 0;

    std::mutex mux;
};

#endif //JMPLAYER_JMAUDIOPLAY_H
