#include "wlegl.h"

#include <android/log.h>

#include "wayland-android-server-protocol.h"

#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "wheatley:wlegl", __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley:wlegl", __VA_ARGS__))

struct wlegl_buffer {
	struct wl_resource *resource;

	android_native_buffer_t native;

	int refcount;
};

static void
wlegl_buffer_incref(struct android_native_base_t* base)
{
	struct wlegl_buffer *buffer;

	buffer = wl_container_of(base, buffer, native);

	__sync_fetch_and_add(&buffer->refcount, 1);
}

static void
wlegl_buffer_decref(struct android_native_base_t* base)
{
	struct wlegl_buffer *buffer;

	buffer = wl_container_of(base, buffer, native);

	if (__sync_fetch_and_sub(&buffer->refcount, 1) > 1)
		return;

	if (buffer->native.handle) {
		native_handle_close((native_handle_t *)buffer->native.handle);
		native_handle_delete((native_handle_t *)buffer->native.handle);
	}

	free(buffer);
}

static void
wlegl_buffer_destroy_handler(struct wl_client *client,
			     struct wl_resource *resource)
{
	wl_resource_destroy(resource);
}

struct wl_buffer_interface wlegl_buffer_implementation = {
	wlegl_buffer_destroy_handler
};

static void
wlegl_buffer_resource_destroyed(struct wl_resource *resource)
{
	struct wlegl_buffer *buffer = wl_resource_get_user_data(resource);

	wlegl_buffer_decref(&buffer->native.common);
}

struct wlegl_buffer *
wlegl_buffer_create(struct wl_resource *parent, uint32_t id,
		    int32_t width, int32_t height, int32_t stride,
		    int32_t format, int32_t usage, struct wlegl_handle *handle)
{
	struct wlegl_buffer *buffer;

	if (!wlegl_handle_is_valid(handle)) {
		wl_resource_post_error(parent, ANDROID_WLEGL_ERROR_BAD_HANDLE,
				       "Invalid handle");
		return;
	}

	buffer = malloc(sizeof *buffer);
	if (!buffer) {
		wl_resource_post_no_memory(parent);
		return;
	}
	memset(buffer, 0, sizeof *buffer);

	buffer->resource =
		wl_resource_create(wl_resource_get_client(parent),
				   &wl_buffer_interface, 1, id);
	if (!buffer->resource) {
		wl_resource_post_no_memory(parent);
		free(handle);
		return;
	}
	wl_resource_set_implementation(buffer->resource,
				       &wlegl_buffer_implementation, buffer,
				       wlegl_buffer_resource_destroyed);

	buffer->native.handle = wlegl_handle_create_native(handle);
	if (!buffer->native.handle) {
		wl_resource_post_no_memory(parent);
		wl_resource_destroy(buffer->resource);
	}

	buffer->native.common.magic = ANDROID_NATIVE_BUFFER_MAGIC;
	buffer->native.common.version = sizeof(buffer->native);
	buffer->native.common.incRef = wlegl_buffer_incref;
	buffer->native.common.decRef = wlegl_buffer_decref;

	buffer->native.width = width;
	buffer->native.height = height;
	buffer->native.stride = stride;
	buffer->native.format = format;
	buffer->native.usage = usage;

	/* The resource holds a reference */
	buffer->refcount = 1;

	return buffer;
}

int
wlegl_buffer_size_func(void *data, struct wl_resource *buffer_res,
		       int32_t *width, int32_t *height)
{
	struct wlegl_buffer *buffer;

	return 0;

	if (!wl_resource_instance_of(buffer_res, &wl_buffer_interface,
				     &wlegl_buffer_implementation))
		return 0;

	buffer = wl_resource_get_user_data(buffer_res);

	*width = buffer->native.width;
	*height = buffer->native.height;
}
