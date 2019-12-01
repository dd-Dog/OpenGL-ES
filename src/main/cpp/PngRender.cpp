//
// Created by bian on 2019/11/29.
//

#include <pthread.h>
#include <unistd.h>
#include "PngRender.h"
#include "android/log.h"

#define LOG_TAG "PngRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


PngRender::PngRender() {
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);
    backLeft = 0;
    backTop = 0;
    backWidth = 720;
    backHeight = 720;
    initial = false;
}

PngRender::~PngRender() {
    pthread_cond_destroy(&mCondition);
    pthread_mutex_destroy(&mLock);
}

bool PngRender::openFile(const char *pngPath) {
    LOGI("open png file:%s", pngPath);
    printids();
    //使用libpng打开PNG图片，并读取出RGBA数据
    pngPicDecoder = new PngPicDecoder();
    pngPicDecoder->openFile(const_cast<char *>(pngPath));

    /**创建子线程进行PNG解码
	 * 参数一 为指向线程标识符的指针
	 * 参数二 定制各种不能的线程属性
	 * 参数三 是线程运行函数的地址
	 * 参数四 是运行函数的参数
	 */
    int ret = pthread_create(&threadId, 0, pngRenderThread, this);
    return ret == 0;
}

void *PngRender::pngRenderThread(void *myself) {
    LOGI("pngRenderThread");
    PngRender *render = (PngRender *) myself;
    render->printids();
    render->renderLoop();
    //用pthread_exit()来调用线程的返回值，用来退出线程，
    // 但是退出线程所占用的资源不会随着线程的终止而得到释放
    pthread_exit(0);
}

void PngRender::renderLoop() {
    LOGI("renderLoop");
    renderEnabled = true;
    while (renderEnabled) {
        LOGI("waiting for mLock...");
        pthread_mutex_lock(&mLock);
        LOGI("Got mLock");
        switch (msg) {
            case MSG_WINDOW_SET: {
                LOGI("MSG_WINDOW_SET");
                //设置了window,开始初始化
                initial = initialize();
                break;
            }
            case MSG_RENDER_LOOP_EXIT:
                LOGI("MSG_RENDER_LOOP_EXIT");
                //退出循环
                renderEnabled = false;
                destroy();
                break;
            case MSG_NONE:
                LOGI("MSG_NONE");
                break;
        }
        msg = MSG_NONE;

        if (initial) {
            LOGI("初始化成功，开始绘制");
            eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
            drawPng();
            pthread_cond_wait(&mCondition, &mLock);
            usleep(100 * 1000);//睡眠100S
        }
        pthread_mutex_unlock(&mLock);
    }
    printids();
}

bool PngRender::initialize() {
    LOGI("initialize");
    EGLint numConfigs;

    /**
     * 1. 初始化EGL环境，为OpenGL和显示设备做适配
     * 2. 创建window
     * 3. 为线程绑定上下文环境
     * 4. 创建纹理并绘制
     * 5. 刷新纹理
     * 6. 渲染到显示设备
     * 7. 创建显卡可执行程序
     */
    const EGLint attribs[] = {EGL_BUFFER_SIZE, 32,
                              EGL_ALPHA_SIZE, 8,
                              EGL_BLUE_SIZE, 8,
                              EGL_GREEN_SIZE, 8,
                              EGL_RED_SIZE, 8,
                              EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                              EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                              EGL_NONE};
    //获取显示设备
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    LOGI("获取并与显示设备连接");
    //初始化EGL
    int major,minor;
    eglInitialize(eglDisplay, &major, &minor);
    LOGI("初始化EGL,对EGL内部数据结构进行设置，返回主版本号和次版本号,major=%d,minor=%d", major, minor);
    //列出并让EGL选择最合适的配置
    eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &numConfigs);
    LOGI("选择合适的配置参数");
    EGLint eglContextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    //构建OpenGL上下文
    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, eglContextAttributes);
    LOGI("构建OpenGL上下文");

    //2.创建window
    EGLint visualID;
    eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &visualID);
    LOGI("查询配置对象中的属性值，原生可视系统的可视ID：EGL_NATIVE_VISUAL_ID=%d", visualID);
    ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, visualID);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, 0);
    LOGI("创建窗口");

    //3.为线程绑定上下文环境
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    LOGI("指定当前上下文，关联特定的EGLContext和渲染表面");

    //4.创建纹理对象并绘制

    //创建纹理对象
    glGenTextures(1, &glTexture);
    //OpenGL与纹理绑定
    glBindTexture(GL_TEXTURE_2D, glTexture);
    //配置纹理过滤器，决定像素如何被填充
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //配置映射规则
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //5.刷新纹理
    //从pngDecoder获取RGBA数据
    const RawImageData rawImageData = pngPicDecoder->getRawImageData();
    LOGI("raw_image_data Meta: width is %d height is %d size is %d colorFormat is %d",
         rawImageData.width, rawImageData.height, rawImageData.size,
         rawImageData.gl_color_format);
    //RGBA数据传给纹理对象
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rawImageData.width, rawImageData.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, (byte *) rawImageData.data);
    pngPicDecoder->releaseRawImageData(&rawImageData);//释放临时对象资源

    //6.渲染到显示设备
    //6.1初始化shader
    //6.1.1 vertext shader的编译
    GLint status;
    glVertexShader = glCreateShader(GL_VERTEX_SHADER);//创建顶点着色器
    glShaderSource(glVertexShader, 1, &VERTEX_SHADER_SOURCE, NULL);//顶点着色器程序加载到内存
    glCompileShader(glVertexShader);//编译顶点着色器程序
    glGetShaderiv(glVertexShader, GL_COMPILE_STATUS, &status);//检查编译是否成功
    if (status == GL_FALSE) {
        glDeleteShader(glVertexShader);
        LOGI("Failed to compile shader : %s\n", VERTEX_SHADER_SOURCE);
        return false;
    }
    //6.1.2 fragment shader的编译
    glFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);//创建片着色器
    glShaderSource(glFragmentShader, 1, &FRAGMENT_SHADER_SOURCE, NULL);//片元着色器加载到内存
    glCompileShader(glFragmentShader);//编译片元着色器
    glGetShaderiv(glFragmentShader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        glDeleteShader(glFragmentShader);
        LOGI("Failed to compile shader : %s\n", FRAGMENT_SHADER_SOURCE);
        return false;
    }
    //6.2 创建显卡可执行程序
    glProgram = glCreateProgram();//创建程序对象
    glAttachShader(glProgram, glVertexShader);//顶点着色器附加到程序
    glAttachShader(glProgram, glFragmentShader);//片元着色器附加到程序
    glBindAttribLocation(glProgram, ATTRIBUTE_VERTEX, "position");
    glBindAttribLocation(glProgram, ATTRIBUTE_TEXCOORD, "texcoord");
    glLinkProgram(glProgram);//链接程序
    glGetProgramiv(glProgram, GL_LINK_STATUS, &status);//检查链接是否成功
    if (status == GL_FALSE) {
        LOGI("Failed to link program %d", glProgram);
        return false;
    }
    glUseProgram(glProgram);

    glUniformSampler = glGetUniformLocation(glProgram, "yuvTexSampler");

    return true;
}

void PngRender::drawPng() {
    LOGI("drawPng");
    //规定窗口大小
    glViewport(backLeft, backTop, backWidth, backHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //使用显卡绘制程序
    glUseProgram(glProgram);

    static const GLfloat vertices[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f};
    //设置顶点坐标
    glVertexAttribPointer(ATTRIBUTE_VERTEX, 2, GL_FLOAT, 0, 0, vertices);
    glEnableVertexAttribArray(ATTRIBUTE_VERTEX);
    //设置纹理坐标
    static const GLfloat texCoords[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    glVertexAttribPointer(ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, 0, 0, texCoords);
    glEnableVertexAttribArray(ATTRIBUTE_TEXCOORD);

    //绑定gltexture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glTexture);
    glUniform1i(glUniformSampler, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(eglDisplay, eglSurface);
}

void PngRender::setWindow(ANativeWindow *window) {
    LOGI("setWindow:%p", window);
    printids();
    pthread_mutex_lock(&mLock);
    nativeWindow = window;
    msg = MSG_WINDOW_SET;
    LOGI("set window success");
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void PngRender::resetSize(int width, int height) {
    LOGI("resetSize,width=%d,height=%d", width, height);
    printids();
    pthread_mutex_lock(&mLock);
    backWidth = width;
    backHeight = height;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void PngRender::stop() {
    LOGI("stop");
    pthread_mutex_lock(&mLock);
    msg = MSG_RENDER_LOOP_EXIT;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);

    LOGI("waiting for render thread end...");
    pthread_join(threadId, 0);
    LOGI("render thread is end");

}

void PngRender::printids() {
    pid_t pid;
    pthread_t tid;
    pid = getpid();
    tid = pthread_self();
    LOGI("print pid and threadNum: pid is %u, threadNum is %u (0x%x)\n", (unsigned int) pid,
         (unsigned int) tid,
         (unsigned int) tid);

}

void PngRender::destroy() {
    LOGI("destroy");
    printids();
    initial = false;
    if (glVertexShader)
        glDeleteShader(glVertexShader);
    if (glFragmentShader)
        glDeleteShader(glFragmentShader);

    glDeleteTextures(1, &glTexture);

    if (glProgram) {
        glDeleteProgram(glProgram);
        glProgram = 0;
    }

    //先销毁surface
    eglDestroySurface(eglDisplay, eglSurface);
    eglSurface = EGL_NO_SURFACE;
    //再销毁Context
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDisplay = EGL_NO_DISPLAY;
    eglContext = EGL_NO_CONTEXT;
}




