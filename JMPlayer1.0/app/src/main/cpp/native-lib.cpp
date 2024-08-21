//
// Created by jiaming.huang on 2024/4/2.
//

#include <jni.h>
#include <string>
#include <android/native_window_jni.h>

#include "JMPlayerPorxy.h"
#include "JMLog.h"

#include "TestOpenglEgl.h"
#include <unistd.h>


static jobject mythiz = NULL;

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_JMPlayer_JMPlayer_testjni(JNIEnv *env, jobject thiz, jstring url, jobject handle) {
    // TODO: implement testjni()
    ALOGE("File %s open failed!",url);
    jclass myjclass = env->FindClass("com/example/JMPlayer/JMPlayer");
    if (myjclass == 0) {
        env->DeleteLocalRef(myjclass);
        ALOGD("jiaming jni post_event jclazz fail  \n");
        return false;
    }

    jmethodID jm = env->GetMethodID(myjclass, "testFanShe", "(Ljava/lang/String;)V");
    if (jm == 0) {
        ALOGD("jiaming jni post_event method fail  \n");
        return false;
    }
    ALOGE("File  open end 2!");

    env->CallVoidMethod(thiz,jm,url);

    //env->DeleteLocalRef(myjclass);
    ALOGE("File  open end!");

    return true;

}

inline static void post_event(JNIEnv *env, jobject thiz,int what, int arg1, int arg2)
{

    jclass myjclass = env->FindClass("com/example/JMPlayer/JMPlayer");
    if (myjclass == 0) {
        env->DeleteLocalRef(myjclass);
        ALOGD("jiaming jni post_event jclazz fail  \n");
        return ;
    }

    jmethodID jm = env->GetMethodID(myjclass, "postEventFromNative", "(III)V");
    if (jm == 0) {
        ALOGD("jiaming jni post_event method fail  \n");
        return ;
    }
    ALOGE("File  open end 2!");

    env->CallVoidMethod(thiz,jm,what, arg1,arg2);

    //env->DeleteLocalRef(myjclass);
    ALOGE("File  open end!");
    return;
}

#define MSG_ERROR                       100     /* arg1 = error */
#define MSG_COMPLETED                   300
#define MSG_VIDEO_SIZE_CHANGED          400

static void message_loop_n(JNIEnv *env)
{
    ALOGD("jiaming jni message_loop_n  \n");
    while(1) {
        AVMessage msg;
        int retval = JMPlayerPorxy::Get()->getMsg(&msg);
        if (retval < 0) {
            ALOGE("jiaming jni message_loop_n retval < 0 \n");
            break;
        }

        switch (msg.what) {
            case MSG_ERROR:
                post_event(env, mythiz,msg.what, msg.arg1, msg.arg2);
                break;

            case MSG_COMPLETED:
                //post_event(JNIEnv *env, jobject thiz,int what, int arg1, int arg2)
                break;

            case MSG_VIDEO_SIZE_CHANGED:
                //post_event(JNIEnv *env, jobject thiz,int what, int arg1, int arg2)
                break;

            default:
                ALOGE("unknown MSG_xxx(%d)\n", msg.what);
                break;
        }

        usleep(1000000);
    }
}

static int message_loop(void *arg)
{
    JNIEnv *env = NULL;
    message_loop_n(env);

    return 0;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_JMPlayer_PlaybackActivity_Open(JNIEnv *env, jobject thiz, jstring url,
                                                jobject handle) {
    // TODO: implement Open()
    const char *myurl = env->GetStringUTFChars(url, 0);
    ALOGD("jiaming jni Java_com_example_JMPlayer_PlaybackActivity_Open %s \n",myurl);

    JMPlayerPorxy::Get()->Open(myurl);
    JMPlayerPorxy::Get()->Start();

    env->ReleaseStringUTFChars(url,myurl);

    return true;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_JMPlayer_JMPlayer_setDataSource(JNIEnv *env, jobject thiz, jstring url,
                                                 jobject handle) {
    // TODO: implement setDataSource()

    const char *myurl = env->GetStringUTFChars(url, 0);

    ALOGD("jiaming jni Java_com_example_JMPlayer_PlaybackActivity_Open %s \n",myurl);
    JMPlayerPorxy::Get()->Open(myurl);
    env->ReleaseStringUTFChars(url,myurl);


    return true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_JMPlayer_JMPlayer_start(JNIEnv *env, jobject thiz) {
    // TODO: implement start()
    JMPlayerPorxy::Get()->Start();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_JMPlayer_JMPlayer_InitView(JNIEnv *env, jobject thiz, jobject surface) {
    // TODO: implement InitView()
    ALOGD("jiaming jni Java_com_example_JMPlayer_JMPlayer_InitView  \n");
    ANativeWindow *win = ANativeWindow_fromSurface(env, surface);

    mythiz = thiz;

    JMPlayerPorxy::Get()->InitView((void *)win);
}

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res)
{
    ALOGD("jiaming jni JNI_OnLoad \n");
    JMPlayerPorxy::Get()->Init(vm, message_loop);

    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_JMPlayer_JMPlayer_Pause(JNIEnv *env, jobject thiz, jboolean is_pause) {
    // TODO: implement Pause()
    JMPlayerPorxy::Get()->Pause(is_pause);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_JMPlayer_JMPlayer_Seek(JNIEnv *env, jobject thiz, jdouble position) {
    // TODO: implement Seek()
    JMPlayerPorxy::Get()->Seek(position);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_JMPlayer_JMPlayer_getDuration(JNIEnv *env, jobject thiz) {
    // TODO: implement getDuration()

    return JMPlayerPorxy::Get()->getDuration();
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_example_JMPlayer_JMPlayer_getCurrentPosition(JNIEnv *env, jobject thiz) {
    // TODO: implement getCurrentPosition()
    return  JMPlayerPorxy::Get()->getCurrentPosition();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_JMPlayer_JMPlayer_getVideoWidth(JNIEnv *env, jobject thiz) {
    // TODO: implement getVideoWidth()
    return JMPlayerPorxy::Get()->getVideoWidth();
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_JMPlayer_JMPlayer_getVideoHeight(JNIEnv *env, jobject thiz) {
    // TODO: implement getVideoHeight()
    return JMPlayerPorxy::Get()->getVideoHeight();
}



