/**
 * Android JNI bridge. Every call from MainActivity is queued onto the GL
 * thread before it reaches this file.
 */

#define LOG_TAG "Fan3D"
#ifndef PLATFORM_ANDROID
#define PLATFORM_ANDROID
#endif

#include <android/asset_manager_jni.h>
#include <jni.h>

#include "Renderer.h"

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_fan3d_MainActivity_nativeInit(
    JNIEnv* environment, jobject, jobject javaAssetManager)
{
    AAssetManager* manager = AAssetManager_fromJava(environment, javaAssetManager);
    if (!manager) {
        LOGE("Unable to obtain Android AssetManager");
        return JNI_FALSE;
    }
    Renderer::Instance().setAssetManager(manager);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeResize(JNIEnv*, jobject, jint width, jint height)
{
    Renderer::Instance().resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeRender(JNIEnv*, jobject)
{
    Renderer::Instance().render();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchDown(JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventDown(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchMove(JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventMove(x, y);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_fan3d_MainActivity_nativeTouchRelease(JNIEnv*, jobject, jfloat x, jfloat y)
{
    Renderer::Instance().TouchEventRelease(x, y);
}
