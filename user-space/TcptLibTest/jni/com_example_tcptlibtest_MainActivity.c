#include <string.h>
#include "com_example_tcptlibtest_MainActivity.h"
#include "libtcpt.h"
#include <jni.h>
  
JNIEXPORT jint JNICALL Java_com_example_tcptlibtest_MainActivity_reg
  (JNIEnv *env, jobject obj, jstring content)
{
     const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    jint ret;

    ret=reg_pwd(str);
        
    (*env)->ReleaseStringUTFChars(env, content, (const char *)str );
//　　return (*env)->NewStringUTF(env, "Hello World! I am Native interface");
    return ret;
}

JNIEXPORT jint JNICALL Java_com_example_tcptlibtest_MainActivity_auth
  (JNIEnv *env, jobject obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    jint ret;

    ret = authen_store_key(str);

    (*env)->ReleaseStringUTFChars(env, content, (const char *)str );
//    return (*env)->NewStringUTF(env, "Hello World! I am Native interface");
    return ret;    
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    void *venv;
    if ((*vm)->GetEnv(vm, (void**)&venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}



