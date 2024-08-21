//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_JMAUDIOPLAY_H
#define JMPLAYER_JMAUDIOPLAY_H

#include "JMObserver.h"
#include "XParameter.h"

#include <list>

class JMAudioPlay:public JMObserver {
public:
    virtual void Update(XData data);
    virtual bool StartPlay(XParameter out) = 0;
    virtual void Close() = 0;
    virtual void Clear();

    virtual XData GetData();
    int maxFrame = 100;
    int pts = 0;
protected:
    std::mutex framesMutex;
    std::list <XData> frames;
};


#endif //JMPLAYER_JMAUDIOPLAY_H
