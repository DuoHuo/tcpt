LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libtestJni

LOCAL_SRC_FILES := $(call all-subdir-c-files)

LOCAL_PRELINK_MODULE := false

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)

LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libcutils \
    libutils \
    liblog \
    libhardware

include $(BUILD_SHARED_LIBRARY)
