//
// Created by jiaming.huang on 2024/4/8.
//

#ifndef JMPLAYER_JMDEMUX_H
#define JMPLAYER_JMDEMUX_H

#include "XData.h"
#include "XThread.h"
#include "JMObserver.h"
#include "XParameter.h"

class JMDemux :public JMObserver {
public:
    virtual bool Open(const char *url) = 0;
    virtual void Close() = 0;
    virtual XData Read() = 0;
    virtual XParameter getVPara() = 0;
    virtual XParameter getAPara() = 0;

    int durationMs = 0;

    virtual bool Seekto(double Position) = 0;

protected:
    virtual void Main();
};


#endif //JMPLAYER_JMDEMUX_H
