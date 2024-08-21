//
// Created by jiaming.huang on 2024/4/27.
//

#include "JMAudioPlay.h"
#include "JMLog.h"


XData  JMAudioPlay::GetData()
{
    XData d;

    isRuning = true;

    while (!isExit) {
        if (IsPause()) {
            XSleep(2);
            continue;
        }

        framesMutex.lock();
        if(!frames.empty()) {
            d = frames.front();
            frames.pop_front();
            framesMutex.unlock();
            pts = d.pts;
            //ALOGD("JMAudioPlay::GetData pts=%d \n",pts);
            return d;
        }
        framesMutex.unlock();
        XSleep(1);
    }

    isRuning = false;

    return d;
}

void JMAudioPlay::Clear()
{
    framesMutex.lock();
    while (!frames.empty()){
        frames.front().Drop();
        frames.pop_front();
    }
    framesMutex.unlock();
}

void JMAudioPlay::Update(XData data)
{
    //ALOGD("JMAudioPlay::Update \n");
    if(data.size <= 0 || !data.data) {
        ALOGE("JMAudioPlay::Update data == MULL fail \n");
        return ;
    }

    while (!isExit) {  //把数据压入队列中
        framesMutex.lock();

        if (frames.size() > maxFrame) {
            framesMutex.unlock();
            XSleep(1);
            continue;
        }
        frames.push_back(data);
        framesMutex.unlock();
        break;
    }
}