LOCAL_PATH := $(call my-dir)/libffi

include $(CLEAR_VARS)

ffi_arch := $(TARGET_ARCH)
ffi_os := linux

include $(LOCAL_PATH)/Libffi.mk

LOCAL_MODULE := libffi

LOCAL_C_INCLUDES :=				\
	$(LOCAL_PATH)/include			\
	$(LOCAL_PATH)/$(ffi_os)-$(ffi_arch)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

include $(BUILD_STATIC_LIBRARY)
