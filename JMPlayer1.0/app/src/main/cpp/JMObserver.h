//
// Created by jiaming.huang on 2024/4/9.
//

#ifndef JMPLAYER_JMOBSERVER_H
#define JMPLAYER_JMOBSERVER_H

#include "XData.h"
#include "XThread.h"

#include <vector>
#include <mutex>

class JMObserver: public XThread{
public:
    virtual void Update(XData data){}
    void AddObs(JMObserver *obs);
    void Notify(XData data);

protected:
    std::vector<JMObserver *>obss;
    std::mutex mux;
};


#endif //JMPLAYER_JMOBSERVER_H
