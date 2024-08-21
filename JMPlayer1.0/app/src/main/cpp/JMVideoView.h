//
// Created by jiaming.huang on 2024/4/15.
//

#ifndef JMPLAYER_JMVIDEOVIEW_H
#define JMPLAYER_JMVIDEOVIEW_H

#include "JMObserver.h"

class JMVideoView :public JMObserver
{
public:
    virtual void SetRender(void *win) = 0;
    virtual void Render(XData data) = 0;
    virtual void Update(XData data) ;
    virtual void Close() = 0;
};


#endif //JMPLAYER_JMVIDEOVIEW_H
