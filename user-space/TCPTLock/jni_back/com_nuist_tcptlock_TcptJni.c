#include "com_nuist_tcptlock_TcptJni.h"
#include <string.h>
#include <jni.h>
#include "libtcpt.h"

/*
 * Class:     com_nuist_tcptlock_TcptJni
 * Method:    reg
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_nuist_tcptlock_TcptJni_reg
  (JNIEnv *env, jclass obj, jstring content)
{
   const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    jint ret;

    ret=reg_pwd(str);                   
    (*env)->ReleaseStringUTFChars(env, content, (const char *)str );
    return ret;
}

/*
 * Class:     com_nuist_tcptlock_TcptJni
 * Method:    auth
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_nuist_tcptlock_TcptJni_auth
  (JNIEnv *env, jclass obj, jstring content)
{
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    jint ret;

    ret = authen_store_key(str);
    (*env)->ReleaseStringUTFChars(env, content, (const char *)str );
    return ret;
}

/*
 * Class:     com_nuist_tcptlock_TcptJni
 * Method:    clearkey
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_nuist_tcptlock_TcptJni_clearkey
  (JNIEnv *env, jclass obj)
{
    return(clear_key());
}

/*
 * Class:     com_nuist_tcptlock_TcptJni
 * Method:    dosomething
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_nuist_tcptlock_TcptJni_dosomething
  (JNIEnv *env, jclass obj, jstring content)
{
    return NULL;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    void *venv;
    if ((*vm)->GetEnv(vm, (void**)&venv, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}
