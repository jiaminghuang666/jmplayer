//
// Created by jiaming.huang on 2024/4/29.
//

#include "JMPlayerPorxy.h"
#include "JMPlayer.h"
#include "JMLog.h"

JMPlayerPorxy *JMPlayerPorxy::Get()
{
    static JMPlayerPorxy px;
    return &px;
}

void JMPlayerPorxy::Init(void *vm ,int(*msg_loop)(void*))
{
    mux.lock();

    if(!player) {
        player = JMPlayer::Get();
    }
    if (vm) {
        player->InitHard(vm);
    }
    player->PlayerBuilder(msg_loop, player);

    mux.unlock();
}


void JMPlayerPorxy::Close()
{
    mux.lock();
    if (player)
        player->Close();
    mux.unlock();
}

int JMPlayerPorxy::Open(const char *myurl)
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->Open(myurl);
    mux.unlock();
    return ret;
}

bool JMPlayerPorxy::Start()
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->Start();
    mux.unlock();
    return ret;
}

void JMPlayerPorxy::Pause(bool isPase)
{
    mux.lock();
    if (player)
        player->Pause(isPase);
    mux.unlock();
}


int JMPlayerPorxy::InitView(void * win)
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->InitView(win);
    mux.unlock();
    return ret;
}

double JMPlayerPorxy::getCurrentPosition()
{
    double ret = 0.0;
    mux.lock();
    if (player)
        ret = player->getCurrentPosition();
    mux.unlock();
    return ret;
}

int JMPlayerPorxy::getDuration()
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->getDuration();
    mux.unlock();

    return ret;
}

bool JMPlayerPorxy::Seek(double Position)
{
    bool ret = false;
    mux.lock();
    if (player)
        ret = player->Seek(Position);
    mux.unlock();
    return ret;
}

int JMPlayerPorxy::getVideoHeight()
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->getVideoHeight();
    mux.unlock();
    return ret;
}

int JMPlayerPorxy::getVideoWidth()
{
    int ret = 0;
    mux.lock();
    if (player)
        ret = player->getVideoWidth();
    mux.unlock();
    return ret;
}


int JMPlayerPorxy::getMsg(AVMessage *msg)
{
    ALOGD("JMPlayerPorxy::getMsg ");
    int ret = 0;
    //mux.lock();
    if (player)
        ret = player->getMsg(msg);
   // mux.unlock();
    return ret;
}