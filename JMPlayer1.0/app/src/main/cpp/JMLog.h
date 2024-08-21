//
// Created by jiaming.huang on 2024/4/2.
//

#ifndef JMPLAYER_JMLOG_H
#define JMPLAYER_JMLOG_H

class JMLog {

};

#ifdef ANDROID
#include <android/log.h>
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"jiaming",__VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,"jiaming",__VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,"jiaming",__VA_ARGS__)
#else
#define ALOGD(...) printf("jiaming",__VA_ARGS__)
#define ALOGI(...) printf("jiaming",__VA_ARGS__)
#define ALOGE(...) printf("jiaming",__VA_ARGS__)
#endif

#endif //JMPLAYER_JMLOG_H
