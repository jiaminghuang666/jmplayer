//
// Created by jiaming.huang on 2024/4/9.
//

#ifndef JMPLAYER_FFDECODE_H
#define JMPLAYER_FFDECODE_H

#include "FFDecode.h"
#include "XData.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <mutex>

#include "JMObserver.h"
#include "XParameter.h"

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

struct  AVCodecContext ;
struct AVFrame;

class FFDecode :public JMDecode{
public:
    FFDecode();
    static void InitHard(void *vm);

    virtual void Clear();
    virtual bool CreateDecode(XParameter para, bool isHard = false);
    virtual void stopDecode();

    bool sendPacket(XData pkt);

    XData receiveFrame();

protected:
    AVCodecContext *cc = 0;
    AVFrame *frame  = 0;
    std::mutex mux;

    long int videoFramecount = 0;
    long int audioFramecount = 0;

   // SwsContext *vctx = 0;
   // char *rgb = new char[1920 * 1080 *4 ];
   //SwrContext *actx = 0;
   //char *pcm = NULL;
};


#endif //JMPLAYER_FFDECODE_H
