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
