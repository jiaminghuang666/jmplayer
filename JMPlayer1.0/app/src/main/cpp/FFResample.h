//
// Created by jiaming.huang on 2024/4/27.
//

#ifndef JMPLAYER_FFRESAMPLE_H
#define JMPLAYER_FFRESAMPLE_H

#include "XParameter.h"
#include "XData.h"
#include "JMResample.h"

#include <mutex>

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
