#ifndef __WHEATLEY_WLEGL_H__
#define __WHEATLEY_WLEGL_H__

#include <wayland-server.h>
#include <libwlb.h>

#include "system/window.h"

struct wlegl *
wlegl_create(struct wl_display *display);
void
wlegl_destroy(struct wlegl *wlegl);

struct wlegl_handle *
wlegl_handle_create(struct wl_resource *parent, uint32_t id,
		    int32_t num_fds, struct wl_array *ints);
buffer_handle_t
wlegl_handle_create_native(struct wlegl_handle *handle);
int
wlegl_handle_is_valid(struct wlegl_handle *handle);

struct wlegl_buffer *
wlegl_buffer_create(struct wl_resource *parent, uint32_t id,
		    int32_t width, int32_t height, int32_t stride,
		    int32_t format, int32_t usage, struct wlegl_handle *handle);

int
wlegl_buffer_size_func(void *data, struct wl_resource *buffer,
		       int32_t *width, int32_t *height);

#endif /* ! defined __WHEATLEY_WLEGL_H__ */
