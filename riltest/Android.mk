LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/../include
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libbinder \
    libcutils
LOCAL_SRC_FILES := riltest.cpp
LOCAL_MODULE := riltest 
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/../include
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libbinder \
    libcutils
LOCAL_SRC_FILES := socket_server.cpp
LOCAL_MODULE := socket_server 
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/../include
LOCAL_SRC_FILES:= \
    Parcel_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libbinder \
    libcutils
LOCAL_MODULE:= parcel_test
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=$(LOCAL_PATH)/../include
LOCAL_SRC_FILES:= \
    attest.c
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libbinder \
    libcutils
LOCAL_MODULE:= attest
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)
