/*
 * Copyright © 2013 Jason Ekstrand
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
 *
 * vim: ts=4 sw=4 sts=4 expandtab
 */
#ifndef __WHEATLEY_WLEGL_PRIVATE_H__
#define __WHEATLEY_WLEGL_PRIVATE_H__

#include "wlegl.h"

#include <hardware/gralloc.h>
#include <system/window.h>

#include <wayland-server.h>

struct wlegl {
	struct wl_global *global;

	struct gralloc_module_t *gralloc;
	struct alloc_device_t *alloc;
};

struct wlegl_handle {
	struct wl_resource *resource;

	struct wl_array ints;
	struct wl_array fds;
	int32_t num_fds;
	int32_t num_ints;

	int buffer_created;
};

struct wlegl_handle *
wlegl_handle_create(struct wl_resource *parent, uint32_t id,
		    int32_t num_fds, struct wl_array *ints);
buffer_handle_t
wlegl_handle_create_native(struct wlegl_handle *handle);

struct wlegl_buffer {
	struct wlegl *wlegl;
	struct wl_resource *resource;

	android_native_buffer_t native;

	int refcount;
};

struct wlegl_buffer *
wlegl_buffer_create(struct wlegl *wlegl, struct wl_resource *parent,
		    uint32_t id, int32_t width, int32_t height, int32_t stride,
		    int32_t format, int32_t usage, struct wlegl_handle *handle);

extern const struct wlb_buffer_type wlegl_buffer_type;

#endif /* ! defined __WHEATLEY_WLEGL_PRIVATE_H__ */
