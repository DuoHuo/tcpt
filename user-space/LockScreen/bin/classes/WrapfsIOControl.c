#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "WrapfsIOControl.h"

JNIEXPORT jint JNICALL Java_WrapfsIOControl_wrapfs_1ioctl(JNIEnv *env, jobject obj)
{
    int fd;
    char buf[10];
    
    const jbyte *str = (const jbyte*)(*env)->GetStringUTFChars(env,content,JNI_FALSE);
    fd = open("/sdcard/test/iotest.txt",O_RDWR);
    if (fd < 0){
        printf("fd open error\n");
        return -1;
    }
    ioctl(fd,174,"hello kernel!\n");
    (*env)->ReleaseStringUTFChars(env,content,(const char*)str);
    return 0;
}


