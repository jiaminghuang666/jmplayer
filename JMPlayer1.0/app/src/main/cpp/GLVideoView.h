//
// Created by jiaming.huang on 2024/4/15.
//

#ifndef JMPLAYER_GLVIDEOVIEW_H
#define JMPLAYER_GLVIDEOVIEW_H

#include "JMVideoView.h"
#include "XTexture.h"
#include <mutex>

class XTexture;

class GLVideoView :public JMVideoView {
public:
    virtual void SetRender(void *win);
    virtual void Render(XData data);

    virtual void Close();
protected:
    void *view = 0;
    XTexture *txt = 0;

    std::mutex mux;
};


#endif //JMPLAYER_GLVIDEOVIEW_H
