#define LOG_TAG "WaylandActivity"
#include "wheatley.h"

#include <android/native_window_jni.h>

#include <stdlib.h>

struct wheatley_activity {
    struct wheatley_compositor *compositor;

    EGLDisplay egl_display;
    struct wlb_gles2_renderer *renderer;

    struct {
        int32_t format, width, height;
	    struct wlb_output *output;
    } output;
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

void
wheatley_activity_update_output(struct wheatley_activity *wa,
        int32_t format, int32_t width, int32_t height)
{
    if (wa->output.output &&
            wa->output.width == width && wa->output.height == height)
        return;

    if (!wa->output.output) {
        wa->output.output = wlb_output_create(wa->compositor->compositor,
                100, 60, "Android", "none");
    }

    wa->output.width = width;
    wa->output.height = height;

    wlb_output_set_mode(wa->output.output, width, height, 60000);
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
        jclass cls, jlong nativeHandle, jobject jsurface)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;
    ANativeWindow *window;
    int32_t format, width, height;

    wheatley_activity_init_egl(wa);

    wa->renderer = wlb_gles2_renderer_create_for_egl(
            wa->compositor->compositor, wa->egl_display, NULL);

    window = ANativeWindow_fromSurface(env, jsurface);
    if (window == NULL) {
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to convert Surface to ANativeWindow");
        return;
    }

    format = ANativeWindow_getFormat(window);
    width = ANativeWindow_getWidth(window);
    height = ANativeWindow_getHeight(window);

    wheatley_activity_update_output(wa, format, width, height);
    wlb_gles2_renderer_add_egl_output(wa->renderer, wa->output.output, window);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_surfaceChangedNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint format, jint width, jint height)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    wheatley_activity_update_output(wa, format, width, height);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_surfaceDestroyedNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    wlb_gles2_renderer_destroy(wa->renderer);
    wa->renderer = NULL;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_repaintNative(
        JNIEnv *env, jclass cls, jlong nativeHandle, jboolean forceRepaint)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    if (wa->output.output == NULL)
        return;

    if (forceRepaint || wlb_output_needs_repaint(wa->output.output))
        wlb_gles2_renderer_repaint_output(wa->renderer, wa->output.output);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_WaylandActivity_repaintFinishedNative(
        JNIEnv *env, jclass cls, jlong nativeHandle, jint timestamp)
{
    struct wheatley_activity *wa =
            (struct wheatley_activity *)(intptr_t)nativeHandle;

    wlb_output_repaint_complete(wa->output.output, timestamp);
    wl_display_flush_clients(wa->compositor->display);
}
