//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_SLAUDIOPLAY_H
#define JMPLAYER_SLAUDIOPLAY_H

#include "JMAudioPlay.h"

#include <mutex>

class SLAudioPlay :public JMAudioPlay {
public:
    virtual bool StartPlay(XParameter out);
    virtual void Close();
    void PlayCall(void *bufq);

    SLAudioPlay();
    virtual ~SLAudioPlay();

protected:
    unsigned  char *buf = 0;

    std::mutex mux;
};


#endif //JMPLAYER_SLAUDIOPLAY_H
