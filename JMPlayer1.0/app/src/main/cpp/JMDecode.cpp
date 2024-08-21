//
// Created by jiaming.huang on 2024/4/10.
//

#include "JMDecode.h"
#include "JMLog.h"

void JMDecode::Update(XData pkt)
{
    //ALOGD("JMDecode::pkt.isAudio %d \n", pkt.isAudio);
    if (pkt.isAudio != isAudio) {
       return;
    }
    while (!isExit) {
        packsMutex.lock();
        if (packs.size() < maxList) {
            packs.push_back(pkt);  //生产者
            packsMutex.unlock();
            break;
        }
        packsMutex.unlock();
        XSleep(1);
    }
}

void JMDecode::Clear()
{
    packsMutex.lock();
    while (!packs.empty()) {
        packs.front().Drop();
        packs.pop_front();
    }
    pts = 0;
    synPts = 0;
    packsMutex.unlock();
}

void JMDecode::Main()
{
    ALOGD("JMDecode::Main enter");
    while (!isExit) {

        if(IsPause()) {
            XSleep(2);
            continue;
        }
        packsMutex.lock();

        if(!isAudio && synPts > 0) {
            //ALOGD("JMDecode::Main synPts=%d pts=%d ",synPts,pts);
            if(synPts < pts ){
                packsMutex.unlock();
                XSleep(1);
                continue;
            }
        }

        if (packs.empty()) {
            packsMutex.unlock();
            XSleep(1);
            continue;
        }

        XData pack = packs.front();  //取出packet 消费者
        packs.pop_front();

        if(this->sendPacket(pack)) {
            while (!isExit) {
                XData frame = receiveFrame();
                if (!frame.data) {
                    ALOGD("receiveFrame frame data is null");
                    break;
                }
                pts = frame.pts;
                //ALOGD("JMDecode::Main pts=%d frame.pts=%d \n",pts,frame.pts);
                //ALOGD("receiveFrame frame size: %d", frame.size);
                this->Notify(frame);
            }
        }

        pack.Drop();
        packsMutex.unlock();
    }
}