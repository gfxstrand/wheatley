#define LOG_TAG "wayland.Seat"
#include "wheatley.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Seat_createNative(JNIEnv *env,
        jclass cls, jlong compositorHandle)
{
    struct wheatley_compositor *wc =
    		(struct wheatley_compositor *)(intptr_t)compositorHandle;
    struct wlb_seat *seat;

    seat = wlb_seat_create(wc->compositor);
    if (seat == NULL) {
        ALOGD("wlb_seat_create failed: %s\n", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to create Seat");
    }

    return (jlong)(intptr_t)seat;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Seat_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_seat *seat = (struct wlb_seat *)(intptr_t)nativeHandle;

    wlb_seat_destroy(seat);
}

// vim: ts=4 sw=4 sts=4 expandtab
