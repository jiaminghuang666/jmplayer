//
// Created by jiaming.huang on 2024/4/9.
//

#ifndef JMPLAYER_FFDECODE_H
#define JMPLAYER_FFDECODE_H

#include "JMDemux.h"
#include "JMDecode.h"
#include "XData.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <mutex>

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


   // SwsContext *vctx = 0;

   // char *rgb = new char[1920 * 1080 *4 ];
   //SwrContext *actx = 0;
   //char *pcm = NULL;
};


#endif //JMPLAYER_FFDECODE_H
