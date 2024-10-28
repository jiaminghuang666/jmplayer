//
// Created by jiaming.huang on 2024/10/30.
//

#ifndef JMPLAYER1_0_UTILS_H
#define JMPLAYER1_0_UTILS_H

#include <sys/time.h>

long long GetNowMs()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int sec = tv.tv_sec%360000;
    long long t = sec*1000 + tv.tv_usec/1000;

    return t;
}

#endif //JMPLAYER1_0_UTILS_H
