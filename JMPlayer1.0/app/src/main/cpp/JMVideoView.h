//
// Created by jiaming.huang on 2024/4/15.
//

#ifndef JMPLAYER_JMVIDEOVIEW_H
#define JMPLAYER_JMVIDEOVIEW_H

#include "JMObserver.h"

#include "JMVideoView.h"
#include "XTexture.h"

#include <mutex>
#include <list>

class JMVideoView :public JMObserver
{
public:
    virtual void EnqueueRender(XData data);
    virtual XData DequeueRender() ;
    virtual void Update(XData data) ;
    virtual void Clear();

    virtual void SetRender(void *win) = 0;
    virtual void Render() = 0;
    virtual void Close() = 0;

    int maxFrame = 30;
protected:
    std::mutex framesMutex;
    std::list <XData> frames;
};



class XTexture;

class GLVideoView :public JMVideoView {
public:
    virtual void SetRender(void *win);
    virtual void Render();

    virtual void Close();
protected:
    void *view = 0;
    XTexture *txt = 0;

    std::mutex mux;
};


#endif //JMPLAYER_JMVIDEOVIEW_H
