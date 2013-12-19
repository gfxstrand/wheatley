WHEATLEY_PATH := $(call my-dir)

include jni/external/libffi.mk
include jni/external/wayland.mk
include jni/external/pixman.mk
include jni/external/libwlb.mk

LOCAL_PATH := $(WHEATLEY_PATH)

# Protocol stuff
WAYLAND_SCANNER_SRC := $(WHEATLEY_PATH)/wayland-android.xml
WAYLAND_SCANNER_DEST := $(LOCAL_PATH)
include $(WHEATLEY_PATH)/external/wayland-scanner.mk

include $(CLEAR_VARS)

LOCAL_MODULE := wheatley
LOCAL_CFLAGS :=
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2
LOCAL_SRC_FILES :=			\
	wayland-android-protocol.c	\
	native_handle.c			\
	wlegl.c				\
	wlegl_handle.c			\
	wlegl_buffer.c			\
	compositor_activity.c
LOCAL_C_INCLUDES = android_headers
LOCAL_STATIC_LIBRARIES := android_native_app_glue wlb

$(LOCAL_PATH)/wayland_android.c: \
	$(LOCAL_PATH)/wayland-android-server-protocol.h

wheatley: wlb

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/test-clients/Android.mk

$(call import-module, android/native_app_glue)
