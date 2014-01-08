/*
 * Copyright © 2012 Collabora, Ltd.
 * Copyright © 2014 Jason Ekstrand
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
#include "wlegl_private.h"

#include <android/log.h>

#include "wayland-android-server-protocol.h"

#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "wheatley:wlegl", __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley:wlegl", __VA_ARGS__))

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
