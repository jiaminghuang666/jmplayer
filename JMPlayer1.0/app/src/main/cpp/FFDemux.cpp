//
// Created by jiaming.huang on 2024/4/8.
//

#include "FFDemux.h"
#include "JMLog.h"

#include "FFDecode.h"
#include "XParameter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0:(double)r.num / (double) r.den;
}

FFDemux::FFDemux()
{
    static bool isFirst = true;
    if (isFirst) {
        isFirst = false;
        av_register_all();   //初始化解封装
        avformat_network_init(); //初始化网络
        avcodec_register_all();
        ALOGD("register FFDemux !!");
    }
}

void FFDemux::Close()
{
    mux.lock();
    if (ic) {
        ALOGD("FFDemux::close avformat_close_input \n");
        avformat_close_input(&ic);
    }

    mux.unlock();
    ALOGD("FFDemux::close success \n");

}

bool FFDemux::Open(const char *url)
{
    int ret = 0;

    Close();

    mux.lock();
    ALOGD("FFDemux::Open url :%s \n",url);
    ret = avformat_open_input(&ic, url, 0, 0); //打开文件
    if (ret != 0 ) {
        mux.unlock();
        ALOGE("FFDemux::Open avformat_open_input %s fail \n",av_err2str(ret) );
        return ret;
    }

    ret = avformat_find_stream_info(ic, 0);
    if (ret != 0) {
        mux.unlock();
        ALOGE("FFDemux::Open avformat_find_stream_info %s fail \n",av_err2str(ret) );
        return ret;
    }


    this->durationMs = calcDuration(ic);
    ALOGD("FFDemux::Open durationMs = %d  \n", this->durationMs );


    for (int i = 0; i < ic->nb_streams; i++) {
        AVStream *as = ic->streams[i];

        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            ALOGD("=== VIDEO stream ===\n" );
            video_index = i;
            ALOGD("FFDemux::Open VIDEO stream: video_index=%d width=%d  height=%d codec_id=%d pixformat=%d  \n",
                  video_index,
                  as->codecpar->width,
                  as->codecpar->height,
                  as->codecpar->codec_id,
                  as->codecpar->format );

            int fps = r2d(as->avg_frame_rate);
            ALOGD("FFDemux::Open VIDEO stream: fps=%d \n",fps );

        }else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            ALOGD("FFDemux::Open === AUDIO stream ===\n" );
            audio_index = i;
            ALOGD("FFDemux::Open AUDIO stream: audio_index =%d sample_rate=%d  channels=%d format=%d \n",
                  audio_index,
                  as->codecpar->sample_rate,
                  as->codecpar->channels,
                  as->codecpar->format);
        }
    }

    mux.unlock();

    getVPara();
    getAPara();
    ALOGD("FFDemux::Open avformat_open_input success \n");

    return true;
}

int FFDemux::calcDuration(AVFormatContext *ic)
{
    int durationMs = 0;
    if (AV_NOPTS_VALUE !=  ic->duration) {
        durationMs = ic->duration / (AV_TIME_BASE/1000);
        ALOGD("FFDemux::Open duration = %lld durationMs =%d  nb_streams = %d \n", ic->duration, durationMs, ic->nb_streams );
    } else {
        //demux 不到duration的时候需要通过文件大小来计算 durationMs = file_size / bit_rate ;  (file_size << 3) M转化bit
        int64_t file_size = avio_size(ic->pb);
        int bit_rate = 0;
        if ( 0 == ic->bit_rate ) {  //ts 流
            for(int i = 0; i < ic->nb_streams; i++) {
                AVStream *as = ic->streams[i];
                if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    bit_rate = ic->streams[i]->codecpar->bit_rate;
                    break;
                }
            }
        } else {
            bit_rate = ic->bit_rate;
        }
        durationMs = (int)(((file_size << 3 ) * 1000000) / bit_rate);  //durationMs = file_size / bit_rate ;  1、 (file_size << 3) M转化bit 2、转换ms * 1000000
        ALOGD("FFDemux::Open file_size = %d  bit_rate = %d durationMs 1=%d  \n", (int)((file_size << 3 ) * 1000),  bit_rate ,durationMs);
    }

    return durationMs;
}


XParameter FFDemux::getVPara()
{
    mux.lock();
    if (!ic) {
        ALOGD("FFDemux::getVPara fail \n");
        mux.unlock();
        return XParameter();
    }

    int ret = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
    if (ret < 0) {
        ALOGD("av_find_best_stream fail \n");
        mux.unlock();
        return XParameter();
    }

    XParameter para;
    para.para = ic->streams[ret]->codecpar;
    video_index = ret;
    ALOGD("video decoder  video_index =%d \n",video_index);
    mux.unlock();
    return para;
}

XParameter FFDemux::getAPara()
{
    mux.lock();
    if (!ic) {
        ALOGD("FFDemux::getVPara fail \n");
        mux.unlock();
        return XParameter();
    }

    int ret = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, 0, 0);
    if (ret < 0) {
        ALOGD("av_find_best_stream fail \n");
        mux.unlock();
        return XParameter();
    }

    XParameter para;
    para.para = ic->streams[ret]->codecpar;
    para.channels = ic->streams[ret]->codecpar->channels;
    para.sample_rate = ic->streams[ret]->codecpar->sample_rate;

    audio_index = ret;
    ALOGD("audio decoder  audio_index =%d \n",audio_index);
    mux.unlock();
    return para;
}



bool FFDemux::Seekto(double Position)
{
    if (Position < 0 || Position > 1) {
        ALOGD("%s Position must seekto 0.0 ~ 1.0 !!", __FUNCTION__ );
        return false;
    }

    bool ret = false;

    mux.lock();
    if (!ic) {
        ALOGD("FFDemux::Seekto fail \n");
        mux.unlock();
        return false;
    }
    avformat_flush(ic);
    long long seekPosition = 0;
    seekPosition = ic->streams[video_index]->duration * Position;
    ALOGD("FFDemux::Seekto seekPosition = %lld  \n",seekPosition);

    //往后跳转到关键帧
    ret = av_seek_frame(ic, video_index, seekPosition, AVSEEK_FLAG_FRAME| AVSEEK_FLAG_BACKWARD);

    mux.unlock();
    return true;
}

XData FFDemux::Read()
{
    int ret =0;

    mux.lock();
    if (!ic) {
        mux.unlock();
        return XData();
    }

    XData d;

    AVPacket *pkt = av_packet_alloc();
    ret = av_read_frame(ic, pkt);
    if (ret != 0 ) {
        ALOGE("Read packet fail !! \n");
        mux.unlock();
        av_packet_free(&pkt);
        return XData();
    }
    //ALOGD("Read data: pack size: %d, pts :%lld ",pkt->size, pkt->pts);
    d.data = (unsigned  char *) pkt;
    d.size = pkt->size;

    if (pkt->stream_index == audio_index) {
        d.isAudio = true;
    }else if (pkt->stream_index == video_index) {
        d.isAudio = false;
    }else {
        mux.unlock();
        av_packet_free(&pkt);
        return XData();
    }

    pkt->pts = pkt->pts * (1000 * r2d(ic->streams[pkt->stream_index]->time_base));
    pkt->dts = pkt->dts * (1000 * r2d(ic->streams[pkt->stream_index]->time_base));

    d.pts = (int)pkt->pts;
    //ALOGD("demux pts : %d \n", d.pts);
    mux.unlock();
    return d;
}

