LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_VENDOR_MODULE = true
LOCAL_SRC_FILES := graphics.c graphics_adf.c graphics_fbdev.c freetype.c graphics_drm.c

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib \
    external/freetype/include \
    system/core/libpixelflinger/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \

LOCAL_WHOLE_STATIC_LIBRARIES += libadf libdrm

LOCAL_MODULE := libftminui

# This used to compare against values in double-quotes (which are just
# ordinary characters in this context).  Strip double-quotes from the
# value so that either will work.

ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),RGBX_8888)
  LOCAL_CFLAGS += -DRECOVERY_RGBX
endif
ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),BGRA_8888)
  LOCAL_CFLAGS += -DRECOVERY_BGRA
endif

$(warning "TARGET_ARCH= $(TARGET_ARCH)")
ifeq ($(strip $(TARGET_ARCH)),arm64)
LOCAL_CFLAGS += -DTARGET_ARCH_ARM64
else
ifeq ($(strip $(TARGET_ARCH)),x86_64)
LOCAL_CFLAGS += -DTARGET_ARCH_x86_64
else
LOCAL_CFLAGS += -DTARGET_ARCH_ARM
endif
endif

ifneq ($(TARGET_RECOVERY_OVERSCAN_PERCENT),)
  LOCAL_CFLAGS += -DOVERSCAN_PERCENT=$(TARGET_RECOVERY_OVERSCAN_PERCENT)
else
  LOCAL_CFLAGS += -DOVERSCAN_PERCENT=0
endif

include $(BUILD_STATIC_LIBRARY)
