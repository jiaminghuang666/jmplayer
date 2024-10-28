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
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    isExit = false;
    threadPause = false;

    thread th(&XThread::ThreadMain, this);
    th.detach();

    return true;
}

void XThread::StopThread()
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
    isExit = true;

    for (int i = 0; i < 200; i++) {
        if (!isRuning) {
            ALOGD("[%s:%d] is running",__func__, __LINE__);
            return;
        }
        XSleep(1);
    }

}

void XThread::SetPause(bool isPause)
{
    ALOGD("[%s:%d] ==enter==",__func__, __LINE__);
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
    ALOGD("ThreadMain enter ");
    isRuning = true;
    Main();
    isRuning = false;
    ALOGD("ThreadMain end ");
}
