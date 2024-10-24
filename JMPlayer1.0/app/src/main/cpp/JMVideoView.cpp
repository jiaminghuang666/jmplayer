//
// Created by jiaming.huang on 2024/4/15.
//

#include "JMVideoView.h"
#include "JMLog.h"


void JMVideoView::EnqueueRender(XData data)
{
    ALOGE("[EnqueueRender %d] \n ", __LINE__);
    if (data.size <=0 || !data.data) {
      ALOGE("[EnqueueRender %d] data == NULL \n ", __LINE__);
      return ;
    }
    while (!isExit) {
       framesMutex.lock();
       if(frames.size() > maxFrame){
          framesMutex.unlock();
          XSleep(1);
          continue;
       }
    frames.push_back(data);
    framesMutex.unlock();
    break;
   }
}

XData JMVideoView::DequeueRender()
{
    ALOGE("[DequeueRender %d] \n ", __LINE__);
    XData d;
    isRuning = true;
    while(!isExit){
       if(IsPause()) {
          XSleep(2);
          continue;
       }

       framesMutex.lock();
       if(!frames.empty()) {
         d = frames.front();
         frames.pop_front();
         framesMutex.unlock();
         return d;
       }
       framesMutex.unlock();
       XSleep(1);
    }

    isRuning = false;

    return d;
}

void JMVideoView::Clear()
{
    framesMutex.lock();
    while (!frames.empty()){
        frames.front().Drop();
        frames.pop_front();
    }
    framesMutex.unlock();
}

void JMVideoView::Update(XData data)
{
    //入显示队列
    //this->Render(data);
    EnqueueRender(data);
}

//================GLVideoView=====================
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
}

void GLVideoView::Render()
{
    XData data = DequeueRender();
    if (!view) {
        ALOGE("GLVideoView::Render view fail \n ");
        return ;
    }
    if (!txt) {
        txt =XTexture::Create();
        txt->Init(view, (XTextureType)data.format);
    }

    ALOGE("GLVideoView::Render Draw:videoFramecount=%lld datas=%p  width=%d  height=%d \n ",data.datas,data.width, data.height,data.videoFramecount );
    txt->Draw(data.datas, data.width, data.height);
}
//================GLVideoView=====================