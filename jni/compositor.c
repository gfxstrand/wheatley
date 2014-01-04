#define LOG_TAG "wheatley:WaylandCompositor"
#include "wheatley.h"

#include <stdlib.h>
#include <errno.h>

#include "jni_util.h"

static int
wlb_log_android_func(enum wlb_log_level level, const char *format, va_list ap)
{
	int priority;

	switch (level) {
	case WLB_LOG_LEVEL_DEBUG:
		priority = ANDROID_LOG_DEBUG;
		break;
	case WLB_LOG_LEVEL_WARNING:
		priority = ANDROID_LOG_WARN;
		break;
	case WLB_LOG_LEVEL_ERROR:
		priority = ANDROID_LOG_ERROR;
		break;
	default:
		priority = ANDROID_LOG_UNKNOWN;
	}

	return __android_log_vprint(priority, "libwlb", format, ap);
}

static void
wl_debug_android_func(const char *format, va_list ap)
{
	__android_log_vprint(ANDROID_LOG_DEBUG, "wayland", format, ap);
}

JNIEXPORT jlong JNICALL
Java_net_jlekstrand_wheatley_Compositor_createNative(JNIEnv *env, jclass cls)
{
    struct wheatley_compositor *wc;

    wc = malloc(sizeof *wc);
    if (wc == NULL) {
        jni_util_throw_by_name(env, "java/lang/OutOfMemoryError", NULL);
        return 0;
    }
    memset(wc, 0, sizeof *wc);

    /* Pass libwlb and libwayland logging through to Android. */
	wlb_log_set_func(wlb_log_android_func);
	wl_debug_set_handler_server(wl_debug_android_func);

    /* XXX: This should be removed at some point */
	setenv("WAYLAND_DEBUG", "server", 1);

    /* XXX: This is hard-coded merely for testing purposes */
	setenv("XDG_RUNTIME_DIR", "/data/data/net.jlekstrand.wheatley/", 1);

	wc->display = wl_display_create();
    if (!wc->display) {
        ALOGD("Failed to create display: %s", strerror(errno));
        goto err_alloc;
    }

	wc->compositor = wlb_compositor_create(wc->display);
    if (!wc->compositor) {
        ALOGD("Failed to create compositor: %s", strerror(errno));
        goto err_display;
    }

	if (wl_display_init_shm(wc->display) < 0) {
        ALOGD("Failed to initialize wl_shm: %s", strerror(errno));
        goto err_compositor;
    }

	wc->wlegl = wlegl_create(wc->compositor);
    if (!wc->wlegl) {
        ALOGD("Failed to initialize android_wlegl: %s", strerror(errno));
        goto err_compositor;
    }

    return (long)(intptr_t)wc;

err_compositor:
    wlb_compositor_destroy(wc->compositor);
err_display:
    wl_display_destroy(wc->display);
err_alloc:
    free(wc);
    jni_util_throw_by_name(env, "java/lang/RuntimeException", NULL);

    return 0;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_Compositor_destroyNative(JNIEnv *env, jclass cls,
        jlong nativeHandle)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)nativeHandle;

    wlegl_destroy(wc->wlegl);
    wlb_compositor_destroy(wc->compositor);
    wl_display_destroy(wc->display);
    free(wc);
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_Compositor_launchClientNative(JNIEnv *env,
        jclass cls, jlong nativeHandle, jbyteArray jcommand)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)nativeHandle;
    struct wl_client *client;
    jint clen;
    char *cmd_str, *args[4];

    clen = (*env)->GetArrayLength(env, jcommand);
    cmd_str = malloc(clen + 1);
    if (!cmd_str) {
        jni_util_throw_by_name(env, "java/lang/OutOfMemoryError", NULL);
        return;
    }

    (*env)->GetByteArrayRegion(env, jcommand, 0, clen, cmd_str);
    if ((*env)->ExceptionCheck(env))
        goto err_alloc;

    cmd_str[clen] = '\0';

    ALOGD("Launching client: %s", cmd_str);

    args[0] = "sh";
    args[1] = "-c";
    args[2] = cmd_str;
    args[3] = NULL;

    client = wlb_compositor_launch_client(wc->compositor,
            "/system/bin/sh", args);
    if (!client) {
        ALOGD("Failed to launch client: %s", strerror(errno));
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Failed to launch client");
        goto err_alloc;
    }

err_alloc:
    free(cmd_str);
}

static int
wheatley_compositor_ALooper_func(int fd, int events, void *data)
{
	struct wheatley_compositor *wc = data;
    ALooper *looper = ALooper_forThread();

    if (!looper || looper != wc->looper) {
        ALOGD("Wrong Looper.  Removing dispatch hook.");
        return 0; /* Not the right looper */
    }

	if (events & ALOOPER_EVENT_INPUT)
		wl_event_loop_dispatch(wl_display_get_event_loop(wc->display), 0);

	wl_display_flush_clients(wc->display);

	return 1;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_Compositor_addToLooperNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)nativeHandle;
    ALooper *looper;

    if (wc->looper) {
        jni_util_throw_by_name(env, "java/lang/IllegalStateException",
                "Compositor already assigned to a looper");
        return;
    }

    looper = ALooper_forThread();
    if (looper == NULL) {
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Current thread does not have a Looper");
        return;
    }

	ALooper_addFd(looper,
            wl_event_loop_get_fd(wl_display_get_event_loop(wc->display)),
            ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT | ALOOPER_EVENT_OUTPUT,
            wheatley_compositor_ALooper_func, wc);

    wc->looper = looper;
}

JNIEXPORT void JNICALL
Java_net_jlekstrand_wheatley_Compositor_removeFromLooperNative(JNIEnv *env,
        jclass cls, jlong nativeHandle)
{
    struct wheatley_compositor *wc =
            (struct wheatley_compositor *)(intptr_t)nativeHandle;
    ALooper *looper;

    if (!wc->looper)
        return;

    looper = ALooper_forThread();
    if (looper == NULL) {
        jni_util_throw_by_name(env, "java/lang/RuntimeException",
                "Current thread does not have a Looper");
        return;
    }

    if (wc->looper != looper) {
        jni_util_throw_by_name(env, "java/lang/IllegalStateException",
                "Compositor is not assigned to this looper");
        return;
    }

    wc->looper = NULL;
    ALooper_removeFd(looper, 
            wl_event_loop_get_fd(wl_display_get_event_loop(wc->display)));
}
