/*
 * Copyright © 2014 Jason Ekstrand
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * vim: ts=4 sw=4 sts=4 expandtab
 */
#define LOG_TAG "wayland.Touch"
#include "wheatley.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_createNative(JNIEnv *env,
        jclass cls, jlong seatHandle)
{
    struct wlb_seat *seat = (struct wlb_seat *)(intptr_t)seatHandle;
    struct wlb_touch *touch;

    touch = wlb_touch_create(seat);
    if (touch == NULL) {
        ALOGD("wlb_touch_create failed: %s\n", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to create Touch");
    }

    return (jlong)(intptr_t)touch;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;

    wlb_touch_destroy(touch);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_downOnOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jint id,
        jlong outputHandle, jfloat x, jfloat y)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    wlb_touch_down_on_output(touch, time, id, output,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_moveOnOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint id,
        jlong outputHandle, jfloat x, jfloat y)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    wlb_touch_move_on_output(touch, id, output,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_finishFrameNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;

    wlb_touch_finish_frame(touch, time);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_upNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jint id)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;

    wlb_touch_up(touch, time, id);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Touch_cancelNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_touch *touch = (struct wlb_touch *)(intptr_t)nativeHandle;

    wlb_touch_cancel(touch);
}
