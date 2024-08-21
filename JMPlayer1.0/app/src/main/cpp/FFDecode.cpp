//
// Created by jiaming.huang on 2024/4/9.
//

#include "FFDecode.h"
#include "JMLog.h"
#include "XParameter.h"
#include "XData.h"

#include <jni.h>


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavcodec/jni.h>
}

long long GetNowMs()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int sec = tv.tv_sec%360000;
    long long t = sec*1000 + tv.tv_usec/1000;

    return t;
}

void FFDecode::InitHard(void *vm)
{
    ALOGD("FFDecode::InitHard");
    av_jni_set_java_vm((JavaVM *)vm, 0);
}

FFDecode::FFDecode()
{

}

void FFDecode::Clear()
{
    JMDecode::Clear();

    mux.lock();
    if (cc)
        avcodec_flush_buffers(cc);
    mux.unlock();
}

void FFDecode::stopDecode()
{
    ALOGD("FFDecode stopDecode");
    mux.lock();
    pts = 0;
    if(frame)
        av_frame_free(&frame);
    if(cc) {
        avcodec_close(cc);
        avcodec_free_context(&cc);
    }
    mux.unlock();
}

bool FFDecode::CreateDecode(XParameter para, bool isHard)
{
    stopDecode();

    if (!para.para) return false;
    AVCodecParameters *p = para.para;

    //打开视频解码器
    AVCodec *codec = avcodec_find_decoder(p->codec_id);
    if (isHard) {
        ALOGE("FFDecode::CreateDecode using  == hardware video decoder == \n");
        codec = avcodec_find_decoder_by_name("h264_mediacodec");
    }

    if (!codec) {
        ALOGE("avcodec_find_decoder failed !!");
        return false;
    }

    mux.lock();
    //解码器初始化
    cc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(cc, p);
    cc->thread_count = 8;


    //打开解码器
    int ret = 0;
    ret = avcodec_open2(cc, 0, 0);
    if (ret != 0) {
        mux.unlock();
        ALOGE("avcodec_open2 failed !!");
        return false;
    }
    ALOGD("open %d decode",cc->codec_type);
    if(cc->codec_type == AVMEDIA_TYPE_VIDEO) {
        // vctx = NULL;
        // outWidth = 1280;
        // outHeight = 720;
        this->isAudio = false;
    } else if (cc->codec_type == AVMEDIA_TYPE_AUDIO) {
        this->isAudio = true;

        /*pcm = new char[48000 * 4 * 2];
        actx = swr_alloc();
        actx = swr_alloc_set_opts(actx,
                                  av_get_default_channel_layout(2),
                                  AV_SAMPLE_FMT_S16,cc->sample_rate,
                                  av_get_default_channel_layout(cc->channels),
                                  cc->sample_fmt,cc->sample_rate,
                                  0,0 );
        ret = swr_init(actx);
        if (ret != 0) {
            ALOGE("swr_init fail !!");
        } else {
            ALOGE("swr_init success !!");
        }*/
    }

    mux.unlock();
    ALOGE("avcodec_open2 success !!");
    return true;
}



bool FFDecode::sendPacket(XData pkt)
{
    int ret = 0;
    //把packet发给解码器
    if (!pkt.data || pkt.size <= 0)  {
        return false;
    }
    mux.lock();
    ret = avcodec_send_packet(cc, (AVPacket *)pkt.data );
    mux.unlock();
    if (ret != 0) {
        ALOGE("avcodec_send_packet fail \n");
        return false;
    }

    return true;
}

XData FFDecode::receiveFrame()
{
    mux.lock();
    int ret;
    int frameCount = 0;
    if (!cc) {
        mux.unlock();
        return XData();
    }

    if (!frame) {
        frame = av_frame_alloc();
    }

    ret = avcodec_receive_frame(cc, frame);
    if (ret != 0) {
        mux.unlock();
        ALOGE("avcodec_receive_frame no more input stream \n");
        return XData();
    }

    XData frameData;
    frameData.data = (unsigned char *) frame;
    if (cc->codec_type == AVMEDIA_TYPE_VIDEO) {
        ALOGD("avcodec_receive_frame decode video pts = %lld",frame->pts);
        frameData.size = (frame->linesize[0] + frame->linesize[1] + frame->linesize[2]) * frame->height;
        frameData.width = frame->width;
        frameData.height = frame->height;

        this->outWidth = frame->width;
        this->outHeight = frame->height;

    } else {
        ALOGD("avcodec_receive_frame decode audio pts = %lld",frame->pts);
        frameData.size =av_get_bytes_per_sample((AVSampleFormat)frame->format ) * frame->nb_samples * 2;
    }

    frameData.format = frame->format;
    if (!isAudio) {
        ALOGD("frameData.format = %d", frameData.format); // 25 nv21
    }

    memcpy(frameData.datas, frame->data, sizeof(frameData.datas));  //这里需要优化，cp一个frame 这个消耗会很大，遇到4k那就更大

    frameData.pts = frame->pts;
    pts = frameData.pts;

    mux.unlock();
    //ALOGD("FFDecode::receiveFrame pts = %d", frameData.pts);
    return frameData;
}

