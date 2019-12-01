//
// Created by bian on 2019/11/29.
//

#ifndef NDKPROJECT_PNGRENDER_H
#define NDKPROJECT_PNGRENDER_H


#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "3rdparty/libpng/png_decoder.h"

static const char* VERTEX_SHADER_SOURCE =
        "attribute vec4 position;    \n"
        "attribute vec2 texcoord;   \n"
        "varying vec2 v_texcoord;     \n"
        "void main(void)               \n"
        "{                            \n"
        "   gl_Position = position;  \n"
        "   v_texcoord = texcoord;  \n"
        "}                            \n";

static const char* FRAGMENT_SHADER_SOURCE =
        "varying highp vec2 v_texcoord;\n"
        "uniform sampler2D yuvTexSampler;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(yuvTexSampler, v_texcoord);\n"
        "}\n";

class PngRender {
private:
    PngPicDecoder *pngPicDecoder;
    bool renderEnabled;
    bool initial;

    pthread_t threadId;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;

    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;

    GLuint glTexture;
    GLuint glVertexShader;
    GLuint glFragmentShader;
    GLuint glProgram;
    GLint glUniformSampler;
    GLint backLeft;
    GLint backTop;
    GLint backWidth;
    GLint backHeight;

    ANativeWindow *nativeWindow;

    enum RenderThreadMessage {
        MSG_NONE = 0, MSG_WINDOW_SET, MSG_RENDER_LOOP_EXIT
    };

    enum {
        ATTRIBUTE_VERTEX, ATTRIBUTE_TEXCOORD,
    };
    enum RenderThreadMessage msg;

public:
    PngRender();
    ~PngRender();
    bool openFile(const char *pngPath);

    void setWindow(ANativeWindow *window);

    void resetSize(int width, int height);

    void stop();

    static void* pngRenderThread(void *myself);

    void renderLoop();

    bool initialize();

    void destroy();

    void printids();

    void drawPng();
};



#endif //NDKPROJECT_PNGRENDER_H
