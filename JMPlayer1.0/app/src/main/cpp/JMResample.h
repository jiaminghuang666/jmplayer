//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_JMRESAMPLE_H
#define JMPLAYER_JMRESAMPLE_H


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


#endif //JMPLAYER_JMRESAMPLE_H
