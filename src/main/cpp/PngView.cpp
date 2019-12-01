//
// Created by bian on 2019/11/29.
//

#include <android/native_window_jni.h>
#include "include/com_flyscale_chapter_4_3_PngView.h"
#include "android/log.h"
#include "PngRender.h"

#ifdef __cplusplus
extern "C" {
#endif
#define LOG_TAG "PngView"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

JNIEXPORT jstring JNICALL Java_com_flyscale_chapter_14_13_PngView_getStringFromJNI
        (JNIEnv *env, jobject jobj, jstring jstr) {
    const char *textFromJava = env->GetStringUTFChars(jstr, NULL);
    LOGI("Receive:%s", textFromJava);
    env->ReleaseStringUTFChars(jstr, textFromJava);
    return env->NewStringUTF("Hello Java,I am JNI!");
}

PngRender *pngRender;
JNIEXPORT void JNICALL Java_com_flyscale_chapter_14_13_PngView_init
        (JNIEnv *env, jobject jobj, jstring pngPath) {
    const char *pngFile = env->GetStringUTFChars(pngPath, NULL);
    pngRender = new PngRender();
    pngRender->openFile(pngFile);
    env->ReleaseStringUTFChars(pngPath, pngFile);
}

ANativeWindow *window;
JNIEXPORT void JNICALL Java_com_flyscale_chapter_14_13_PngView_setSurface
        (JNIEnv *env, jobject jobj, jobject surface) {
    if (surface != 0 && NULL != pngRender) {
        //JVM传过来的Surface转换为NativeWindow
        window = ANativeWindow_fromSurface(env, surface);
        LOGI("Got window %p", window);// %p:打印指针的值
        //设置window对象
        pngRender->setWindow(window);
    } else if (window == 0) {
        LOGI("Release Window");
        ANativeWindow_release(window);
        window = 0;
    }
}


JNIEXPORT void JNICALL Java_com_flyscale_chapter_14_13_PngView_resetSize
        (JNIEnv *env, jobject jobj, jint width, jint height) {
    if (NULL != pngRender) {
        pngRender->resetSize(width, height);
    }
}

JNIEXPORT void JNICALL Java_com_flyscale_chapter_14_13_PngView_stop
        (JNIEnv *env, jobject jobj) {
    if (NULL!= pngRender){
        pngRender->stop();
        delete pngRender;
        pngRender = NULL;
    }
}

#ifdef __cplusplus
}
#endif