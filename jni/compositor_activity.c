#include <jni.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <EGL/egl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <libwlb.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "wheatley", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "wheatley", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley", __VA_ARGS__))

struct wheatley_activity {
	struct android_app *app;

	struct wl_display *display;
	struct wl_event_loop *display_loop;
	struct wlb_compositor *compositor;
	struct wlb_gles2_renderer *renderer;

	EGLDisplay egl_display;

	struct wlb_output *output;
};

static void
activity_create_outputs(struct wheatley_activity *activity)
{
	int32_t width, height;

	width = ANativeWindow_getWidth(activity->app->window);
	height = ANativeWindow_getHeight(activity->app->window);

	LOGD("Creating %dx%d Output", width, height);
	activity->output = wlb_output_create(activity->compositor,
					     100, 60, "Android", "none");
	wlb_output_set_mode(activity->output, width, height, 60000);
}

static void
activity_init_window(struct wheatley_activity *activity)
{
	if (activity->app->window == NULL)
		return;

	if (!activity->output)
		activity_create_outputs(activity);

	LOGD("Initializing EGL");
	activity->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(activity->egl_display, 0, 0);

	activity->renderer =
		wlb_gles2_renderer_create_for_egl(activity->compositor,
						  activity->egl_display, NULL);
	wlb_gles2_renderer_add_egl_output(activity->renderer,
					  activity->output,
					  activity->app->window);
}

static void
activity_term_window(struct wheatley_activity *activity)
{
	wlb_gles2_renderer_destroy(activity->renderer);
	activity->renderer = NULL;
	eglTerminate(activity->egl_display);
	activity->egl_display = EGL_NO_DISPLAY;
}

static void
activity_repaint(struct wheatley_activity *activity)
{
	struct timeval tv;
	uint32_t timestamp;

	if (!activity->renderer)
		return;

	wlb_gles2_renderer_repaint_output(activity->renderer, activity->output);

	gettimeofday(&tv, NULL);
	timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	wlb_output_repaint_complete(activity->output, timestamp);

	wl_display_flush_clients(activity->display);
}

/**
 * Process the next input event.
 */
static int32_t
activity_handle_input(struct android_app* app, AInputEvent* event)
{
	struct wheatley_activity *activity =
		(struct wheatley_activity *)app->userData;
	return 0;
}

/**
 * Process the next main command.
 */
static void
activity_handle_cmd(struct android_app* app, int32_t cmd)
{
	struct wheatley_activity *activity =
		(struct wheatley_activity *)app->userData;

	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		break;
	case APP_CMD_INIT_WINDOW:
		activity_init_window(activity);
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		activity_term_window(activity);
		break;
	case APP_CMD_GAINED_FOCUS:
		break;
	case APP_CMD_LOST_FOCUS:
		break;
	}
}

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
		LOGD("default log");
		priority = ANDROID_LOG_UNKNOWN;
	}

	return __android_log_vprint(priority, "libwlb", format, ap);
}

static void
wl_debug_android_func(const char *format, va_list ap)
{
	__android_log_vprint(ANDROID_LOG_DEBUG, "wayland", format, ap);
}

static int
display_dispatch_func(int fd, int events, void *data)
{
	struct wheatley_activity *activity = data;

	if (events & ALOOPER_EVENT_INPUT)
		wl_event_loop_dispatch(activity->display_loop, 0);
	if (events & ALOOPER_EVENT_OUTPUT)
		wl_display_flush_clients(activity->display);

	return 1;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void
android_main(struct android_app* app)
{
	struct wheatley_activity activity;
	struct android_poll_source* source;
	int ident, events;
	char *args[] = {
		"su",
		"-c",
		"fedora.sh -u jason weston --backend=wayland-backend.so",
		NULL
	};

	/* Make sure glue isn't stripped. */
	app_dummy();

	memset(&activity, 0, sizeof(activity));
	app->userData = &activity;
	app->onAppCmd = activity_handle_cmd;
	app->onInputEvent = activity_handle_input;
	activity.app = app;

	if (app->savedState != NULL) {
		/* XXX: We should restore from saved state. */
	}

	wlb_log_set_func(wlb_log_android_func);
	wl_debug_set_handler_server(wl_debug_android_func);

	setenv("WAYLAND_DEBUG", "server", 1);

	activity.display = wl_display_create();
	activity.display_loop = wl_display_get_event_loop(activity.display);
	activity.compositor = wlb_compositor_create(activity.display);

	wl_display_init_shm(activity.display);

	setenv("XDG_RUNTIME_DIR", "/data/data/net.jlekstrand.wheatley/", 1);
	wlb_compositor_launch_client(activity.compositor,
				     "/system/xbin/su", args);

	ALooper_addFd(ALooper_forThread(),
		      wl_event_loop_get_fd(activity.display_loop),
		      ALOOPER_POLL_CALLBACK,
		      ALOOPER_EVENT_INPUT | ALOOPER_EVENT_OUTPUT,
		      display_dispatch_func, &activity);

	/* loop waiting for stuff to do. */

	while (1) {
		/* If not animating, we will block forever waiting for
		 * events.  If animating, we loop until all events are
		 * read, then continue to draw the next frame of animation.
		 */
		while ((ident=ALooper_pollAll(activity.renderer ? 0 : -1,
					      NULL, &events,
					      (void**)&source)) >= 0) {

			/* Process this event. */
			if (source != NULL) {
				source->process(app, source);
			}

			/* Check if we are exiting. */
			if (app->destroyRequested != 0) {
				/* engine_term_display(&engine); */
				return;
			}
		}

		if (activity.renderer)
			activity_repaint(&activity);
        }
}
