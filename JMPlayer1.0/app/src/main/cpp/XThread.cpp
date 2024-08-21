//
// Created by jiaming.huang on 2024/4/8.
//

#include "XThread.h"
#include "JMLog.h"

#include <thread>
using namespace std;

void XSleep(int Ms)
{
    chrono::milliseconds du(Ms);
    this_thread::sleep_for(du);
}

bool XThread::StartThread()
{
    ALOGD("StartThread enter 1");
    isExit = false;
    threadPause = false;

    thread th(&XThread::ThreadMain, this);
    th.detach();
    ALOGD("StartThread enter 2");
    return true;
}

void XThread::StopThread()
{
    ALOGD("StopThread enter ");
    isExit = true;

    for (int i = 0; i < 200; i++) {
        if (!isRuning) {
            ALOGD("StopThread ok ");
            return;
        }
        XSleep(1);
    }
    ALOGD("StopThread end ");
}

void XThread::SetPause(bool isPause)
{
     threadPause = isPause;
     for ( int i = 0; i < 10; i++) {

         if (isPausing == isPause){
             break;
         }
         XSleep(10);
     }

}

bool  XThread::IsPause()
{
    isPausing = threadPause;
    return threadPause;
}

void XThread::ThreadMain()
{
    isRuning = true;
    ALOGD("ThreadMain enter ");
    Main();
    ALOGD("ThreadMain end ");
    isRuning = false;
}
