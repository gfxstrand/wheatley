#define LOG_TAG "wayland.Pointer"
#include "wheatley.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_createNative(JNIEnv *env,
        jclass cls, jlong seatHandle)
{
    struct wlb_seat *seat = (struct wlb_seat *)(intptr_t)seatHandle;
    struct wlb_pointer *pointer;

    pointer = wlb_pointer_create(seat);
    if (pointer == NULL) {
        ALOGD("wlb_pointer_create failed: %s\n", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to create Pointer");
    }

    return (jlong)(intptr_t)pointer;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_destroyNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wlb_pointer *pointer = (struct wlb_pointer *)(intptr_t)nativeHandle;

    wlb_pointer_destroy(pointer);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_enterOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jlong outputHandle, jfloat x, jfloat y)
{
    struct wlb_pointer *pointer = (struct wlb_pointer *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    wlb_pointer_enter_output(pointer, output,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_moveOnOutputNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jlong outputHandle,
        jfloat x, jfloat y)
{
    struct wlb_pointer *pointer = (struct wlb_pointer *)(intptr_t)nativeHandle;
    struct wlb_output *output = (struct wlb_output *)(intptr_t)outputHandle;

    wlb_pointer_move_on_output(pointer, time, output,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_buttonNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jint button,
        jboolean pressed)
{
    struct wlb_pointer *pointer = (struct wlb_pointer *)(intptr_t)nativeHandle;

    wlb_pointer_button(pointer, time, button,
            pressed ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_wayland_Pointer_axisNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jint time, jfloat xval, jfloat yval)
{
    struct wlb_pointer *pointer = (struct wlb_pointer *)(intptr_t)nativeHandle;
    wl_fixed_t fx, fy;

    fx = wl_fixed_from_double(xval);
    fy = wl_fixed_from_double(yval);

    if (fx != 0)
        wlb_pointer_axis(pointer, time, WL_POINTER_AXIS_HORIZONTAL_SCROLL, fx);

    if (fy != 0)
        wlb_pointer_axis(pointer, time, WL_POINTER_AXIS_VERTICAL_SCROLL, fy);
}

// vim: ts=4 sw=4 sts=4 expandtab
