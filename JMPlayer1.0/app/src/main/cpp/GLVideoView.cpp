//
// Created by jiaming.huang on 2024/4/15.
//

#include "GLVideoView.h"
#include "JMLog.h"

void GLVideoView::Close()
{
    mux.lock();

   if (txt) {
       txt->Drop();
       txt = 0;
   }

   mux.unlock();
}

void GLVideoView::SetRender(void *win)
{
    ALOGE("GLVideoView::SetRender enter \n ");
    view = win;
    ALOGE("GLVideoView::SetRender out \n ");
}

void GLVideoView::Render(XData data)
{
    ALOGE("GLVideoView::Render enter \n ");
    if (!view) {
        ALOGE("GLVideoView::Render view fail \n ");
        return ;
    }

    if (!txt) {
        txt =XTexture::Create();
        txt->Init(view, (XTextureType)data.format);
    }

    ALOGE("GLVideoView::Render Draw:datas=%p  width=%d  height=%d \n ",data.datas,data.width, data.height );
    txt->Draw(data.datas, data.width, data.height);

    ALOGE("GLVideoView::Render out \n ");
}