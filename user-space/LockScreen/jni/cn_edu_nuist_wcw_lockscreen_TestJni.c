#include <jni.h>
#include <stdio.h>
#include "cn_edu_nuist_wcw_lockscreen_TestJni.h"
#include <android/log.h>
#define LOG_TAG "cqEmbed"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

JNIEXPORT void JNICALL Java_cn_edu_nuist_wcw_lockscreen_TestJni_print
  (JNIEnv *env, jobject obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    int ret;

    ret = authen_store_key(str);
    if (ret < 0) {
    	LOGI("anthen_store_key error\n");
	return;
    }
    LOGI("OOOOOOOOOOO:send storekey cmd\n");
    (*env)->ReleaseStringUTFChars(env,content,(const char*)str);
    return;
}

JNIEXPORT void JNICALL Java_cn_edu_nuist_wcw_lockscreen_TestJni_brint
  (JNIEnv *env, jobject obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    int ret;

    ret = clear_key();
    if (ret < 0) {
    	LOGI("clear_key error\n");
	return;
    }
    LOGI("OOOOOOOOOOO:send clearkey cmd\n");
    (*env)->ReleaseStringUTFChars(env,content,(const char*)str);
    return;
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    void *venv;
    if ((*vm)->GetEnv(vm, (void**)&venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

  
