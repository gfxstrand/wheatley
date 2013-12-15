LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := demo-activity
LOCAL_CFLAGS :=
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_SRC_FILES := demo_activity.c
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
