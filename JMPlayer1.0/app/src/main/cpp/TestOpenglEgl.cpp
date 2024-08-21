//
// Created by jiaming.huang on 2024/4/17.
//

#include "TestOpenglEgl.h"
#include "JMLog.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <string.h>

#include <android/native_window.h>
#include <EGL/egl.h>

//yuv 元数据的宽高
int width = 424;
int height = 240;



//
EGLDisplay display = EGL_NO_DISPLAY;

//对应的window 设置
EGLSurface winsurface = EGL_NO_SURFACE;

//顶点着色器glsl
#define GET_STR(x) #x
static const char *vertexShader = GET_STR(
        attribute vec4 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
        vTexCoord = vec2(aTexCoord.x,1.0-aTexCoord.y);
            gl_Position = aPosition;
        }
);

//片元着色器,
static const char *fragYUV420P = GET_STR(
    precision mediump float;    //精度
        varying vec2 vTexCoord;
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;
        void main() {
            vec3 yuv ;
            vec3 rgb;
            yuv.r = texture2D(yTexture, vTexCoord).r;
            yuv.g = texture2D(uTexture, vTexCoord).r - 0.5;
            yuv.b = texture2D(vTexture, vTexCoord).r - 0.5;
            rgb = mat3(1.0, 1.0, 1.0,
                       0.0,-0.39465,2.03211,
                       1.13983,-0.58060,0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);

GLint InitShader(const char * code, GLint type)
{
    GLint sh = glCreateShader(type);
    if ( sh == 0 ) {
        ALOGE("glCreateShader fail %d !! \n", type);
        return 0;
    }

    glShaderSource(sh,
                   1,
                   &code,
                   0);
    glCompileShader(sh);

    GLint status;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        ALOGE("glGetShaderiv fail !! \n");
        return 0;
    }
    ALOGD("glCompileShader success ! \n");
    return sh;

}


std::int16_t Testdisplayer(const char * url, void * windows)
{
    ALOGD("Testdisplayer: url = %s ", url);

    FILE *fp = fopen(url, "rb");
    if (!fp) {
        ALOGE("open file %s failed !!", url);
        return false;
    }

    ANativeWindow * nwin = (ANativeWindow *) windows;
    // ========EGL=======
    //1、egl display 创建和初始化
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        ALOGE("eglGetDisplay failed !!");
        return false;
    }
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        ALOGE("eglInitialize failed !!");
        return false;
    }

    // 2 配置surface
    EGLConfig config;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };
    if(EGL_TRUE != eglChooseConfig(display,configSpec,&config,1,&configNum))
    {
        ALOGE("eglChooseConfig failed!");
        return false;
    }

     winsurface = eglCreateWindowSurface(display,config,nwin,0);
    if (winsurface == EGL_NO_SURFACE) {
        ALOGE("eglCreateWindowSurface test failed !!");
        return false;
    }

    //3 context 创建关联的上下文
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE
    };
    EGLContext context = eglCreateContext(display,config,EGL_NO_CONTEXT,ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        ALOGE("eglCreateContext failed !!");
        return false;
    }
    if (EGL_TRUE != eglMakeCurrent(display, winsurface, winsurface, context)) {
        ALOGE("eglMakeCurrent fail");
        return false;
    }
    ALOGE("EGL Init Success!");

    // ========EGL=======
    initOpenGL(fp);  // init OpenGL
    //displayYUV(fp);

}

std::int16_t initOpenGL(FILE *fp)
{
    // =====openGL=======
    //顶点 和 片源 初始化shader
    GLint vsh = InitShader(vertexShader, GL_VERTEX_SHADER);
    GLint fsh = InitShader(fragYUV420P, GL_FRAGMENT_SHADER);

    //创建渲染程序
    GLint program = glCreateProgram();
    if (program == 0) {
        ALOGE("glGetShaderiv fail !! \n");
        return 0;
    }

    //渲染程序中加入着色器代码
    glAttachShader(program, vsh);
    glAttachShader(program,fsh);

    //链接程序
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        ALOGE("glGetProgramiv fail !! \n");
        return 0;
    }

    glUseProgram(program);
    ALOGD(" ===glUseProgram sucess == !! \n");

    //加入三维顶点数据
    static float vers[] = {
            1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, 1.0f,0.0f,
            -1.0f,1.0f,0.0f,
    };
    GLuint apos = (GLuint) glGetAttribLocation(program, "aPosition");
    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 12, vers);

    static float txts[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0, 1.0
    };
    GLuint  atex = (GLuint) glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(atex);
    glVertexAttribPointer(atex, 2, GL_FLOAT,GL_FALSE, 8, txts);


    glUniform1i(glGetUniformLocation(program,"yTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "uTexture"), 1);
    glUniform1i(glGetUniformLocation(program, "vTexture"), 2);

    ALOGD(" ===glUseProgram sucess 1 == !! \n");

    //创建opengl纹理
    GLuint texts[3] = {0};

    //创建三个纹理
    //设置纹理属性
    glGenTextures(3,texts);
    ALOGD(" ===glUseProgram sucess 2 == !! \n");
    glBindTexture(GL_TEXTURE_2D, texts[0]);
    //缩小的过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //设置纹理的格式和大小
    glTexImage2D(GL_TEXTURE_2D,
                 0,//细节基本 0默认
                 GL_LUMINANCE,//gpu内部格式 亮度，灰度图
                 width, height, //拉升到全屏
                 0,  //边框
                 GL_LUMINANCE, //数据的像素格式 亮度，灰度图 要与上面一致
                 GL_UNSIGNED_BYTE, //像素的数据类型
                 NULL                    //纹理的数据
    );

    ALOGD(" ===glUseProgram sucess 3 == !! \n");
    glBindTexture(GL_TEXTURE_2D,texts[1]);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 width/2,height/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 NULL
    );


    glBindTexture(GL_TEXTURE_2D,texts[2]);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 width/2,height/2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 NULL
    );

    ALOGD(" ===openGL sucess == !! \n");
    // =====openGL=======


    // =====displayYUV=======
    unsigned  char * buf[3] = {0};
    buf[0] = new unsigned char[width * height];
    buf[1] = new unsigned char[width * height / 4];
    buf[2] = new unsigned char[width * height /4];

    for (int i = 0; i < 10000; i++) {
        if(feof(fp) == 0) {
            fread(buf[0], 1, width * height, fp);
            fread(buf[1], 1,width * height / 4, fp);
            fread(buf[2], 1,width * height / 4, fp);
        }


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texts[0]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf[0]);

        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,texts[1]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_LUMINANCE,GL_UNSIGNED_BYTE,buf[1]);


        //激活第2层纹理,绑定到创建的opengl纹理
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,texts[2]);
        //替换纹理内容
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_LUMINANCE,GL_UNSIGNED_BYTE,buf[2]);

        //绘制
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示
        eglSwapBuffers(display, winsurface);
    }

    // =====displayYUV=======
}



