#ifndef __WHEATLEY_WHEATLEY_H__
#define __WHEATLEY_WHEATLEY_H__

#include <jni.h>
#include <android/log.h>
#include <android/looper.h>
#include <libwlb.h>

#include "wlegl.h"

#ifndef LOG_TAG
#   define LOG_TAG "wheatley:unknown"
#endif

#define ALOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define ALOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

struct wheatley_output {
    struct wlb_output *output;
};

struct wheatley_compositor {
    struct wl_display *display;
    struct wlb_compositor *compositor;
    struct wlegl *wlegl;

    ALooper *looper;

    struct wl_list output_list;
};

#endif /* ! defined __WHEATLEY_WHEATLEY_H__ */
