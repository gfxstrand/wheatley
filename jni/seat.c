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
 */
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

/* vim: set ts=4 sw=4 sts=4 expandtab: */
