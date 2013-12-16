#include <jni.h>
#include <errno.h>

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

	struct wlb_output *output;
};

static void
activity_create_outputs(struct wheatley_activity *activity)
{
	int32_t width, height;
	EGLDisplay display;

	LOGD("Initializing EGL");
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);

	width = ANativeWindow_getWidth(activity->app->window);
	height = ANativeWindow_getHeight(activity->app->window);
	LOGD("Creating %dx%d Output", width, height);
	activity->output = wlb_output_create(activity->compositor,
					     100, 60, "Android", "none");
	activity->renderer =
		wlb_gles2_renderer_create_for_egl(activity->compositor,
						  display, NULL);
	wlb_output_set_mode(activity->output, width, height, 60000);
	wlb_gles2_renderer_add_egl_output(activity->renderer,
					  activity->output,
					  activity->app->window);
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
		// The window is being shown, get it ready.
		if (activity->app->window != NULL) {
			activity_create_outputs(activity);
			/*
			engine_init_display(engine);
			engine_draw_frame(engine);
			*/
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		/* engine_term_display(engine); */
		break;
	case APP_CMD_GAINED_FOCUS:
		break;
	case APP_CMD_LOST_FOCUS:
		break;
	}
}

static int
wlb_android_log_func(enum wlb_log_level level, const char *format, va_list ap)
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

static int
display_dispatch_func(int fd, int events, void *data)
{
	struct wheatley_activity *activity = data;

	wl_display_flush_clients(activity->display);
	wl_event_loop_dispatch(activity->display_loop, 0);

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

	wlb_log_set_func(wlb_android_log_func);

	activity.display = wl_display_create();
	activity.display_loop = wl_display_get_event_loop(activity.display);
	activity.compositor = wlb_compositor_create(activity.display);

	wl_display_init_shm(activity.display);

	ALooper_addFd(ALooper_prepare(0),
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
		while ((ident=ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {

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
			wlb_gles2_renderer_repaint_output(activity.renderer,
							  activity.output);
        }
}
