//
// Created by jiaming.huang on 2024/4/9.
//


#include "JMObserver.h"

void JMObserver::AddObs(JMObserver *obs)
{
    if(!obs) return;
    mux.lock();
    obss.push_back(obs);
    mux.unlock();
}

void JMObserver::Notify(XData data)
{
    mux.lock();
    for(int i = 0; i < obss.size(); i++) {
        obss[i]->Update(data);
    }
    mux.unlock();
}