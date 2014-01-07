#define LOG_TAG "wayland.Output"
#include "wheatley.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Output_createNative(JNIEnv *env,
        jclass cls, jlong compositorHandle,
        jint physicalWidth, jint physicalHeight)
{
    struct wheatley_compositor *wc =
    		(struct wheatley_compositor *)(intptr_t)compositorHandle;
    struct wlb_output *output;

    output = wlb_output_create(wc->compositor, physicalWidth, physicalHeight,
            "Android", "none");
    if (output == NULL) {
        ALOGD("wlb_output_create failed: %s\n", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to create Output");
    }

    return (jlong)(intptr_t)output;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Output_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_output *output = (struct wlb_output *)(intptr_t)nativeHandle;

    wlb_output_destroy(output);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Output_setModeNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint width, jint height, jint refresh)
{
    struct wlb_output *output = (struct wlb_output *)(intptr_t)nativeHandle;

    wlb_output_set_mode(output, width, height, refresh);
}

// vim: ts=4 sw=4 sts=4 expandtab
