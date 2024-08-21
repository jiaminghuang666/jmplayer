//
// Created by jiaming.huang on 2024/4/29.
//

#ifndef JMPLAYER_JMPLAYERPORXY_H
#define JMPLAYER_JMPLAYERPORXY_H

#include "JMPlayer.h"
#include <mutex>

class JMPlayerPorxy :public JMPlayer{
public:
    static JMPlayerPorxy * Get();
    //{
    //    static JMPlayerPorxy px;
    //    return &px;
    //}
     void Init(void *vm = 0, int(*msg_loop)(void*) = NULL);

    virtual int Open(const char *myurl);
    virtual void Close();
    virtual bool Start();
    virtual void Pause(bool isPase);

    virtual int InitView(void * win);

    virtual double getCurrentPosition();
    virtual int getDuration();
    virtual bool Seek(double Position);

    virtual int getVideoWidth();
    virtual int getVideoHeight();

    virtual int getMsg(AVMessage *msg);
protected:
    JMPlayerPorxy(){};
    JMPlayer *player = 0;
    std::mutex mux;
};


#endif //JMPLAYER_JMPLAYERPORXY_H
