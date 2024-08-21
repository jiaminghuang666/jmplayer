//
// Created by jiaming.huang on 2024/4/15.
//

#include "XEGL.h"
#include <android/native_window.h>
#include <EGL/egl.h>

#include "JMLog.h"
#include <mutex>

class CXEGL:public XEGL {


public:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;

    std::mutex mux;

    virtual void Draw()
    {
        mux.lock();
        ALOGD("======= eglSwapBuffers  frame ======");
        if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
            mux.unlock();
            ALOGE("======= Draw frame fail: NO display & NO surface ======");
            return;
        }

        eglSwapBuffers(display, surface);
        mux.unlock();
    }

    virtual void Close()
    {
        mux.lock();
        if (display == EGL_NO_DISPLAY) {
            mux.unlock();
            return;
        }
        eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);

        if (display != EGL_NO_SURFACE)
            eglDestroySurface(display,surface);
        if (context != EGL_NO_CONTEXT)
            eglDestroyContext(display,context);

        eglTerminate(display);

        display = EGL_NO_DISPLAY;
        surface = EGL_NO_SURFACE;
        context = EGL_NO_CONTEXT;

        mux.unlock();
    }

    virtual bool Init(void *win)
    {
        ANativeWindow *nwin = (ANativeWindow *) win;

        Close();

        mux.lock();
        //1 获取EGLDisplay对象 显示设备
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            mux.unlock();
            ALOGE("eglGetDisplay failed !");
            return false;
        }
        ALOGD("eglGetDisplay success !");

        //2 初始化Display
        if (EGL_TRUE != eglInitialize(display, 0, 0)) {
            mux.unlock();
            ALOGE("eglInitialize failed !");
            return false;
        }
        ALOGD("eglInitialize success !");

        //3 获取配置并创建surface
        EGLint configSpec [] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_SURFACE_TYPE,EGL_WINDOW_BIT,
                EGL_NONE
        };

        EGLConfig config = 0;
        EGLint numConfigs = 0;
        if (EGL_TRUE != eglChooseConfig(display,configSpec, &config, 1, &numConfigs)) {
            mux.unlock();
            ALOGE("eglChooseConfig failed !");
            return false;
        }
        ALOGD("eglChooseConfig success !");
        surface = eglCreateWindowSurface(display, config, nwin, NULL);
        if (surface == EGL_NO_SURFACE) {
            mux.unlock();
            ALOGE("eglCreateWindowSurface test failed !!");
            return false;
        }
        //4 创建并打开EGL上下文
        const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);
        if (context == EGL_NO_CONTEXT) {
            mux.unlock();
            ALOGE("eglCreateContext failed !");
            return false;
        }
        ALOGD("eglCreateContext success !");

        if (EGL_TRUE != eglMakeCurrent(display, surface, surface,context)) {
            mux.unlock();
            ALOGE("eglMakeCurrent failed !");
            return false;
        }
        ALOGD("eglMakeCurrent success !");

        mux.unlock();
        return true;
    }
};




XEGL *XEGL::Get()
{
    static CXEGL egl;
    return &egl;
}