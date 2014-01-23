/*
 * Copyright © 2013-2014 Jason Ekstrand
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
#define LOG_TAG "wheatley:wayland.Renderer"
#include "wheatley.h"

#include <android/native_window_jni.h>

#include <stdlib.h>

struct wheatley_renderer {
    EGLDisplay egl_display;
    struct wlb_gles2_renderer *renderer;
};

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Renderer_createNative(JNIEnv *env,
        jclass cls, jlong compositorHandle)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)compositorHandle;
    struct wheatley_renderer *wr;

    wr = malloc(sizeof *wr);
    if (wr == NULL) {
        jni_util_throw_by_name(env, "java/lang/OutOfMemoryError", NULL);
        return 0;
    }
    memset(wr, 0, sizeof *wr);

    wr->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(wr->egl_display, 0, 0);
    wr->renderer = wlb_gles2_renderer_create_for_egl(wc->compositor,
            wr->egl_display, NULL);

    return (intptr_t)wr;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Renderer_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_renderer *wr =
            (struct wheatley_renderer *)(intptr_t)nativeHandle;

    if (wr->renderer)
        wlb_gles2_renderer_destroy(wr->renderer);

    eglTerminate(wr->egl_display);

    free(wr);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Renderer_addOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jlong outputHandle, jobject jsurface)
{
    struct wheatley_renderer *wr =
            (struct wheatley_renderer *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;
    ANativeWindow *window;

    window = ANativeWindow_fromSurface(env, jsurface);
    if (window == NULL) {
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to convert Surface to ANativeWindow");
        return;
    }

    wlb_gles2_renderer_add_egl_output(wr->renderer, output, window);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Renderer_removeOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jlong outputHandle)
{
    struct wheatley_renderer *wr =
            (struct wheatley_renderer *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    /* wlb_gles2_renderer_remove_egl_output(wr->renderer, output); */
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Renderer_repaintOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jlong outputHandle,
        jboolean forceRepaint)
{
    struct wheatley_renderer *wr =
            (struct wheatley_renderer *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    if (forceRepaint || wlb_output_needs_repaint(output))
        wlb_gles2_renderer_repaint_output(wr->renderer, output);
}

/* vim: set ts=4 sw=4 sts=4 expandtab: */
