//
// Created by jiaming.huang on 2024/4/27.
//

#include "JMResample.h"


void JMResample::Update(XData data)
{
    XData d = this->Resample(data);
    if (d.size > 0) {
        this->Notify(d);
    }
}