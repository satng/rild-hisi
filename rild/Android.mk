# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES :=$(LOCAL_PATH)/../include

LOCAL_SRC_FILES:= \
	rild.c


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libril_sp

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

LOCAL_CFLAGS := -DRIL_SHLIB

LOCAL_MODULE:= rild
# LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

# For radiooptions binary
# =======================
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	radiooptions.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \

LOCAL_CFLAGS := \

LOCAL_MODULE:= radiooptions_sp
# LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
