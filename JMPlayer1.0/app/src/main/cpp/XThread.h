//
// Created by jiaming.huang on 2024/4/8.
//

#ifndef JMPLAYER_XTHREAD_H
#define JMPLAYER_XTHREAD_H

void XSleep(int Ms);

class XThread {
public:
    virtual bool StartThread();
    virtual void StopThread();

    virtual void Main() {};

    virtual void SetPause(bool isPause);
    virtual bool IsPause();

protected:
    bool isExit = false;
    bool isRuning = false;

    bool threadPause = false;
    bool isPausing  = false;

private:
    void ThreadMain();

};


#endif //JMPLAYER_XTHREAD_H
