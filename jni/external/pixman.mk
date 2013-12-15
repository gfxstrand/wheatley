LOCAL_PATH := $(call my-dir)

PIXMAN_PATH := $(LOCAL_PATH)/pixman/pixman

include $(PIXMAN_PATH)/Android.mk

__ndk_modules.pixman.EXPORT_C_INCLUDES += $(PIXMAN_PATH)
