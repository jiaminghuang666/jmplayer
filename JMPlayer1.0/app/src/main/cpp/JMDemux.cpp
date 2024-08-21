//
// Created by jiaming.huang on 2024/4/8.
//

#include "JMDemux.h"
#include "JMLog.h"

void JMDemux::Main()
{
    ALOGD("JMDemux::Main enter");
    while (!isExit) {
        if (IsPause()) {
            XSleep(2);
            continue;
        }

        XData d = Read();
        if(d.size > 0)
            Notify(d);
        else
            XSleep(2);

        //ALOGD("read data size is %d", d.size);
        //if (d.size <= 0) break;
    }
}