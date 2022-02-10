ifneq ($(TARGET_SIMULATOR),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
commands_local_path := $(LOCAL_PATH)

# display language LANGUAGE_CN/LANGUAGE_EN(Chinese or English)
LOCAL_CFLAGS += -DLANGUAGE_CN

LOCAL_CFLAGS += -DHAL_FLASH_FUN

LOCAL_C_INCLUDES    +=  $(LOCAL_PATH) \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/ui/include \
    $(LOCAL_PATH)/testitem \
    $(LOCAL_PATH)/testitem/include \
    $(LOCAL_PATH)/modem \
    $(LOCAL_PATH)/chnmgr \
    $(LOCAL_PATH)/testitem/common/storage

LOCAL_C_INCLUDES    +=  external/tinyalsa/include \
    hardware/libhardware/include/hardware \
    system/connectivity/ \
    system/core/include/ \
    system/connectivity/wificond \
    frameworks/opt/net/wifi/libwifi_hal/include \
    external/wpa_supplicant_8/wpa_supplicant/wpa_client_include \
    vendor/sprd/proprietories-source/engpc/sprd_fts_inc \
    bootable/recovery \
    bootable/recovery/include \

LOCAL_STATIC_LIBRARIES := \
    libftminui \
    libpng \
    libz \
    android.hardware.sensors@1.0-convert \
    libgmock \
    libbootloader_message \
    libfstab

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    libhardware_legacy \
    libtinyalsa \
    libcutils \
    liblog \
    libdl \
    libbase \
    libnetutils \
    libutils \
    libbinder \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    android.hardware.sensors@1.0 \
    libmemion \
    android.hardware.wifi@1.0 \
    libwpa_client

LOCAL_SRC_FILES := factorytest.cpp \
    parse_conf.cpp

LOCAL_SRC_FILES += $(call all-cpp-files-under, modem)
LOCAL_SRC_FILES += $(call all-cpp-files-under, testitem)
LOCAL_SRC_FILES += $(call all-cpp-files-under, chnmgr)
LOCAL_SRC_FILES += $(call all-cpp-files-under, ui)

#copy audio_sample.pcm
$(shell mkdir -p $(PRODUCT_OUT)/vendor/media)
$(shell cp -rf $(LOCAL_PATH)/audio_sample.pcm $(PRODUCT_OUT)/vendor/media/audio_sample.pcm)

#copy libft2.so to vendor/lib
LOCAL_POST_INSTALL_CMD := $(hide) \
    cp -rf $(LOCAL_PATH)/lib_factorytest_ft2.so $(PRODUCT_OUT)/vendor/lib/lib_factorytest_ft2.so; \

LOCAL_INIT_RC := factorytest.rc
LOCAL_MODULE := factorytest
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS += -fstack-protector-all
LOCAL_REQUIRED_MODULES := NotoSansCJK-Regular-sprd.ttc

include $(BUILD_EXECUTABLE)
include $(commands_local_path)/ui/minui/Android.mk
include $(commands_local_path)/etc/Android.mk
endif

