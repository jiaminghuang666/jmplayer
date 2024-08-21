//
// Created by jiaming.huang on 2024/4/23.
//

#ifndef JMPLAYER_XSHADER_H
#define JMPLAYER_XSHADER_H

#include <mutex>

enum XShaderType {
    XSHADER_YUV420P =0,
    XSHADER_NV12 = 25,
    XSHADER_NV21 = 26
};

class XShader {

public:
    virtual bool Init(XShaderType type=XSHADER_YUV420P);
    virtual void Close();
    virtual void Draw();
    virtual void GetTexture(unsigned int index,int width,int height, unsigned char *buf,bool isa = false);

protected:
    unsigned int vsh = 0;
    unsigned int fsh = 0;
    unsigned int program = 0;
    unsigned int texts[100] = {0};
    std::mutex mux;
};


#endif //JMPLAYER_XSHADER_H
