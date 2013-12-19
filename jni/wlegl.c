#include "wlegl.h"

#include <stdlib.h>
#include <android/log.h>

#include "wayland-android-server-protocol.h"

#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "wheatley:wlegl", __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley:wlegl", __VA_ARGS__))

struct wlegl {
	struct wl_global *global;
};

static void 
wlegl_create_handle(struct wl_client *client, struct wl_resource *resource,
		    uint32_t id, int32_t num_fds, struct wl_array *ints)
{
	wlegl_handle_create(resource, id, num_fds, ints);
}

static void 
wlegl_create_buffer(struct wl_client *client, struct wl_resource *resource,
		    uint32_t id, int32_t width, int32_t height, int32_t stride,
		    int32_t format, int32_t usage, struct wl_resource *handle)
{
	wlegl_buffer_create(resource, id, width, height, stride, format, usage,
			    wl_resource_get_user_data(handle));
}

struct android_wlegl_interface wlegl_interface = {
	wlegl_create_handle,
	wlegl_create_buffer
};

static void
wlegl_bind(struct wl_client *client,
	   void *data, uint32_t version, uint32_t id)
{
	struct wl_resource *resource;

	resource = wl_resource_create(client, &android_wlegl_interface, 1, id);
	if (!resource) {
		wl_client_post_no_memory(client);
		return;
	}

	wl_resource_set_implementation(resource, &wlegl_interface, NULL, NULL);
}

struct wlegl *
wlegl_create(struct wl_display *display)
{
	struct wlegl *wlegl;

	wlegl = malloc(sizeof *wlegl);
	if (!wlegl) {
		ALOGE("Failed to allocate wlegl");
		return NULL;
	}
	wlegl->global = wl_global_create(display, &android_wlegl_interface,
					 1, NULL, wlegl_bind);
}

void
wlegl_destroy(struct wlegl *wlegl)
{
	wl_global_destroy(wlegl->global);
	free(wlegl);
}
