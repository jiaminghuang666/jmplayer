//
// Created by jiaming.huang on 2024/4/8.
//

#ifndef JMPLAYER_FFDEMUX_H
#define JMPLAYER_FFDEMUX_H

#include "JMDemux.h"
#include "FFDecode.h"

#include <mutex>

struct AVFormatContext;
struct  AVCodecContext;

typedef struct MediaInfo {
    int duration;

}MediaInfo;

class FFDemux: public JMDemux {
public:
    FFDemux();
    virtual bool Open(const char *url);
    virtual void Close();
    virtual XData Read();
    virtual XParameter getVPara();
    virtual XParameter getAPara();

    bool Seekto(double Position);

private:
    AVFormatContext *ic = 0;

    FFDecode *ffdecode;

    AVCodecContext *vc = 0;
    AVCodecContext *ac = 0;
    int video_index = 0;
    int audio_index = 0;

    MediaInfo *mediainfo;

    std::mutex mux;

    virtual int calcDuration(AVFormatContext *ic);

};


#endif //JMPLAYER_FFDEMUX_H
