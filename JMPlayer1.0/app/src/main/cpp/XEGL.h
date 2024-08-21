//
// Created by jiaming.huang on 2024/4/15.
//

#ifndef JMPLAYER_XEGL_H
#define JMPLAYER_XEGL_H


class XEGL {
public:
    virtual bool Init(void *win) = 0;
    virtual void Close() = 0;
    static XEGL *Get();

    virtual void Draw() = 0;
protected:
    XEGL(){}
};


#endif //JMPLAYER_XEGL_H
