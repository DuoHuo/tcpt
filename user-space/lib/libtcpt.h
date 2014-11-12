#ifndef LIBTCPT_H
#define LIBTCPT_H
#include "md5.h"
#include "sha1.h"
#include <android/log.h>
#define LOG_TAG "TCPT"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

int reg_pwd(char *pwd);
int authen_store_key(char *pwd);
int clear_key();
#define MD5_PATH "/sdcard/.shadow"
#endif 
