# Copyright 2008 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    reference-cdma-sms.c

LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libril_sp

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include $(KERNEL_HEADERS)

LOCAL_SHARED_LIBRARIES += \
  libcutils libutils
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE:= libreference-cdma-sms
include $(BUILD_SHARED_LIBRARY)
