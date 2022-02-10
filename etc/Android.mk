LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

$(warning "src : $(TARGET_OUT_VENDOR_ETC)")
LOCAL_MODULE := NotoSansCJK-Regular-sprd.ttc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES := NotoSansCJK-Regular-sprd.ttc

include $(BUILD_PREBUILT)