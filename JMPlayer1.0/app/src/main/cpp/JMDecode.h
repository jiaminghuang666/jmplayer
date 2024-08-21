//
// Created by jiaming.huang on 2024/4/10.
//

#ifndef JMPLAYER_JMDECODE_H
#define JMPLAYER_JMDECODE_H

#include "JMObserver.h"
#include "XParameter.h"
#include "XData.h"

#include <list>

class JMDecode :public JMObserver{
public:
    virtual bool CreateDecode(XParameter para, bool isHard = false) = 0;
    virtual bool sendPacket(XData pkt) = 0;
    virtual XData receiveFrame() = 0;

    virtual void stopDecode() = 0;

    virtual void Clear();
    void Update(XData data);

    bool isAudio = false;
    //最大的队列缓冲
    int maxList = 100;

    int synPts = 0;
    int pts = 0;

    int outWidth = 0;
    int outHeight = 0;

protected:
    virtual void Main();

     std::list<XData> packs;
     std::mutex packsMutex;
};


#endif //JMPLAYER_JMDECODE_H
