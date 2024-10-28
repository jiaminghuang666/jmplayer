//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_FFRESAMPLE_H
#define JMPLAYER_FFRESAMPLE_H

#include "XParameter.h"
#include "XData.h"

#include <mutex>

#include "JMObserver.h"
#include "XParameter.h"

class JMResample :public JMObserver {
public:
    virtual bool Open(XParameter in, XParameter out) = 0;
    virtual XData Resample(XData indata) = 0;
    virtual void Update(XData data);
    int outChannels = 2;
    int outFormat = 1;
};


struct SwrContext;
class FFResample :public JMResample {
public:
    virtual bool Open(XParameter in,XParameter out);
    virtual void Close();
    XData Resample(XData indata);

protected:
    SwrContext *actx = 0;
    std::mutex mux;
};


#endif //JMPLAYER_FFRESAMPLE_H
