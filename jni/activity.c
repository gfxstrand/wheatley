#define LOG_TAG "WaylandActivity"
#include "wheatley.h"

#include <android/native_window_jni.h>

#include <stdlib.h>

struct wheatley_activity {
    struct wheatley_compositor *compositor;

    EGLDisplay egl_display;
    struct wlb_gles2_renderer *renderer;
};

void
wheatley_activity_init_egl(struct wheatley_activity *wa)
{
    if (wa->egl_display != EGL_NO_DISPLAY)
        return;

    ALOGD("Initializing EGL");
    wa->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(wa->egl_display, 0, 0);
}

void
wheatley_activity_finish_egl(struct wheatley_activity *wa)
{
    if (wa->egl_display == EGL_NO_DISPLAY)
        return;

    ALOGD("Terminating EGL");
    eglTerminate(wa->egl_display);
    wa->egl_display = EGL_NO_DISPLAY;
}

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_createNative(JNIEnv *env,
        jclass cls, jlong compositorHandle)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)compositorHandle;
    struct wheatley_activity *wa;

    wa = malloc(sizeof *wa);
    if (wa == NULL) {
        jni_util_throw_by_name(env, "java/lang/OutOfMemoryError", NULL);
        return 0;
    }
    memset(wa, 0, sizeof *wa);

    wa->compositor = wc;
    wa->egl_display = EGL_NO_DISPLAY;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    if (wa->renderer)
        wlb_gles2_renderer_destroy(wa->renderer);

    wheatley_activity_finish_egl(wa);

    free(wa);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_surfaceCreatedNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jlong outputHandle, jobject jsurface)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;
    ANativeWindow *window;

    wheatley_activity_init_egl(wa);

    wa->renderer = wlb_gles2_renderer_create_for_egl(
            wa->compositor->compositor, wa->egl_display, NULL);

    window = ANativeWindow_fromSurface(env, jsurface);
    if (window == NULL) {
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to convert Surface to ANativeWindow");
        return;
    }

    wlb_gles2_renderer_add_egl_output(wa->renderer, output, window);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_surfaceDestroyedNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    if (wa->renderer) {
        wlb_gles2_renderer_destroy(wa->renderer);
        wa->renderer = NULL;
    }
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_repaintNative(
        JNIEnv *env, jclass cls, jlong nativeHandle, jlong outputHandle,
        jboolean forceRepaint)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    if (forceRepaint || wlb_output_needs_repaint(output))
        wlb_gles2_renderer_repaint_output(wa->renderer, output);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_repaintFinishedNative(
        JNIEnv *env, jclass cls, jlong nativeHandle, jlong outputHandle,
        jint timestamp)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    wlb_output_repaint_complete(output, timestamp);
    wl_display_flush_clients(wa->compositor->display);
}

// vim: ts=4 sw=4 sts=4 expandtab
