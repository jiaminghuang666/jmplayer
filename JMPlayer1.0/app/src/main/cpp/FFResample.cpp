//
// Created by jiaming.huang on 2024/4/27.
//

#include "FFResample.h"
#include "JMLog.h"

extern "C"
{
#include <libswresample/swresample.h>

}
#include <libavcodec/avcodec.h>

void FFResample::Close()
{
    mux.lock();
    if(actx) {
        swr_free(&actx);
    }
    mux.unlock();
}

bool FFResample::Open(XParameter in,XParameter out)
{
    Close();

    mux.lock();
    actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(out.channels),
                              AV_SAMPLE_FMT_S16,out.sample_rate,
                              av_get_default_channel_layout(in.para->channels),
                              (AVSampleFormat)in.para->format,in.para->sample_rate,
                              0,0);
    int re = swr_init(actx);
    if (re != 0) {
        mux.unlock();
        ALOGE("FFRsample::Open swr_init fail !! \n");
        return false;
    }

    outChannels = in.para->channels;
    outFormat = AV_SAMPLE_FMT_S16;

    ALOGD("FFRsample::Open swr_init ==success==  \n");
    mux.unlock();
    return true;
}

XData FFResample::Resample(XData indata)
{
    mux.lock();
    if (indata.size <= 0 || !indata.data) {
        mux.unlock();
        ALOGE("FFRsample::Resample indata NULL fail !! \n");
        return XData();
    }
    if (!actx) {
        mux.unlock();
        ALOGE("FFRsample::Resample actx NULL fail !! \n");
        return XData();
    }

    AVFrame *frame = (AVFrame *)indata.data;

    //ALOGE("indata size is %d \n",indata.size);
    //输出数据
    XData out;
    int outsize = outChannels * frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)outFormat);
    if (outsize <= 0){
        ALOGE("FFRsample::Resample outsize fail !! \n");
        return XData();
    }
    out.Alloc(outsize);
    uint8_t *outArr[2] = {0};
    outArr[0] = out.data;
    int len = swr_convert(actx, outArr, frame->nb_samples, (const uint8_t **)frame->data, frame->nb_samples);
    if (len <= 0) {
        mux.unlock();
        out.Drop();
        return XData();
    }

    out.pts = indata.pts;
    mux.unlock();
    //ALOGE("FFRsample::Resample pts %d  !! \n",out.pts);
    return out;

}
