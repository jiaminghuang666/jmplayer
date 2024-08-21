//
// Created by jiaming.huang on 2024/4/10.
//

#ifndef JMPLAYER_XPARAMETER_H
#define JMPLAYER_XPARAMETER_H

struct AVCodecParameters;

class XParameter {
public:
    AVCodecParameters *para = 0;
    int channels = 2;
    int sample_rate = 44100;
};


#endif //JMPLAYER_XPARAMETER_H
