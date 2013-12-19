#include "wlegl.h"

#include <android/log.h>

#include "wayland-android-server-protocol.h"

#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "wheatley:wlegl", __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley:wlegl", __VA_ARGS__))

struct wlegl_handle {
	struct wl_resource *resource;

	struct wl_array ints;
	struct wl_array fds;
	int32_t num_fds;
	int32_t num_ints;

	int buffer_created;
};

static void
wlegl_handle_add_fd(struct wl_client *client, struct wl_resource *resource,
		    int32_t fd)
{
	struct wlegl_handle *handle = wl_resource_get_user_data(resource);
	int *next_fd;

	if (handle->fds.size >= handle->num_fds * sizeof(int)) {
		wl_resource_post_error(handle->resource,
				       ANDROID_WLEGL_HANDLE_ERROR_TOO_MANY_FDS,
				       "too many file descriptors");
		close(fd);
		return;
	}

	next_fd = wl_array_add(&handle->fds, sizeof(int));
	if (!next_fd) {
		wl_resource_post_no_memory(resource);
		return;
	}

	*next_fd = fd;
}

static void
wlegl_handle_destroy(struct wl_client *client, struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

struct android_wlegl_handle_interface wlegl_handle_implementation = {
	wlegl_handle_add_fd,
	wlegl_handle_destroy
};

static void
destroy_wlegl_handle(struct wl_resource *resource)
{
	struct wlegl_handle *handle = wl_resource_get_user_data(resource);
	int *fd;

	wl_array_for_each(fd, &handle->fds)
		close(fd);

	wl_array_release(&handle->ints);
	wl_array_release(&handle->fds);

	free(handle);
}

struct wlegl_handle *
wlegl_handle_create(struct wl_resource *parent, uint32_t id,
		    int32_t num_fds, struct wl_array *ints)
{
	struct wlegl_handle *handle;

	handle = malloc(sizeof *handle);
	if (!handle) {
		wl_resource_post_no_memory(parent);
		return;
	}
	memset(handle, 0, sizeof *handle);

	handle->resource =
		wl_resource_create(wl_resource_get_client(parent),
				   &android_wlegl_handle_interface, 1, id);
	if (!handle->resource) {
		wl_resource_post_no_memory(parent);
		free(handle);
		return;
	}
	wl_resource_set_implementation(handle->resource,
				       &wlegl_handle_implementation,
				       handle, destroy_wlegl_handle);

	wl_array_init(&handle->ints);
	wl_array_init(&handle->fds);

	wl_array_copy(&handle->ints, ints);

	handle->num_fds = num_fds;
	handle->num_ints = ints->size / sizeof(int);
}

int
wlegl_handle_is_valid(struct wlegl_handle *handle)
{
	return handle->fds.size == (handle->num_fds * sizeof(int));
}

buffer_handle_t
wlegl_handle_create_native(struct wlegl_handle *handle)
{
	native_handle_t *native;

	native = native_handle_create(handle->num_fds, handle->num_ints);
	if (!native)
		return NULL;

	memcpy(&native->data[0], handle->fds.data, handle->fds.size);
	memcpy(&native->data[handle->num_fds], handle->ints.data,
	       handle->ints.size);

	handle->fds.size = 0;

	return native;
}
