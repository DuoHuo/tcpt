#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include "cn_edu_nuist_wcw_lockscreen_TestJni.h"
#include <android/log.h>
#define LOG_TAG "cqEmbed"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

JNIEXPORT void JNICALL Java_cn_edu_nuist_wcw_lockscreen_TestJni_print
  (JNIEnv *env, jobject obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    int file_desc;
    char data128[128];

    file_desc = open("/dev/keymanager",0);
    if (file_desc < 0) {
    	LOGI("Cannot open device file.\n");
    }
    
    memset(data128, 65, 128);
    ioctl(file_desc, 0xee, data128);
    LOGI("OOOOOOOOOOO:send storekey cmd\n");
    (*env)->ReleaseStringUTFChars(env,content,(const char*)str);
    return;
}

JNIEXPORT void JNICALL Java_cn_edu_nuist_wcw_lockscreen_TestJni_brint
  (JNIEnv *env, jobject obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    int file_desc;
	
    file_desc = open("/dev/keymanager",0);
    if (file_desc < 0) {
    	LOGI("Cannot open device file.\n");
    }
    ioctl(file_desc, 0xef, NULL);
    LOGI("OOOOOOOOOOO:send clearkey cmd\n");
    (*env)->ReleaseStringUTFChars(env,content,(const char*)str);
    return;
}


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    void *venv;
    LOGI("OOOOOOOOXXXXXXXXXXOOOOOOOOOOload");
    if ((*vm)->GetEnv(vm, (void**)&venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}

  
