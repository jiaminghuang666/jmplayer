//
// Created by jiaming.huang on 2024/4/8.
//

#ifndef JMPLAYER_FFDEMUX_H
#define JMPLAYER_FFDEMUX_H

#include "XData.h"
#include "XThread.h"
#include "JMObserver.h"
#include "XParameter.h"


#include "FFDecode.h"
#include "FFDemux.h"

#include <mutex>

struct AVFormatContext;
struct  AVCodecContext;

typedef struct MediaInfo {
    int duration;

}MediaInfo;

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
