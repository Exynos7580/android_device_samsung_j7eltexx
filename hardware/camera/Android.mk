LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_STATIC_LIBRARIES := libbase libarect
LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libutils libcutils \
    android.hidl.token@1.0-utils \
    android.hardware.graphics.bufferqueue@1.0

LOCAL_HEADER_LIBRARIES := libnativebase_headers

LOCAL_MODULE                := camera.universal7580
LOCAL_MODULE_RELATIVE_PATH  := hw
LOCAL_MODULE_TAGS           := optional

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

include $(BUILD_SHARED_LIBRARY)
