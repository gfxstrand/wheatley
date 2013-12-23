#include "wlegl_private.h"

#include <android/log.h>

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "wayland-android-server-protocol.h"

#define ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "wheatley:wlegl", __VA_ARGS__))
#define ALOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "wheatley:wlegl", __VA_ARGS__))

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
		buffer->wlegl->gralloc->unregisterBuffer(buffer->wlegl->gralloc,
							 buffer->native.handle);
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
wlegl_buffer_create(struct wlegl *wlegl, struct wl_resource *parent,
		    uint32_t id, int32_t width, int32_t height, int32_t stride,
		    int32_t format, int32_t usage, struct wlegl_handle *handle)
{
	struct wlegl_buffer *buffer;
	int err;

	if (handle->fds.size != (handle->num_fds * sizeof(int))) {
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

	buffer->wlegl = wlegl;
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

	err = wlegl->gralloc->registerBuffer(wlegl->gralloc,
					     buffer->native.handle);
	if (err) {
		ALOGD("Error registering buffer");
		wl_resource_post_error(parent, ANDROID_WLEGL_ERROR_BAD_HANDLE,
				       "Invalid handle");
		wl_resource_destroy(buffer->resource);
	}

	buffer->native.common.magic = ANDROID_NATIVE_BUFFER_MAGIC;
	buffer->native.common.version = sizeof(struct ANativeWindowBuffer);
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

static int
wlegl_buffer_is_type(void *data, struct wl_resource *buffer_res)
{
	return wl_resource_instance_of(buffer_res, &wl_buffer_interface,
				       &wlegl_buffer_implementation);
}

static void
wlegl_buffer_get_size(void *data, struct wl_resource *buffer_res,
		      int32_t *width, int32_t *height)
{
	struct wlegl_buffer *buffer;

	buffer = wl_resource_get_user_data(buffer_res);

	*width = buffer->native.width;
	*height = buffer->native.height;
}

static void
wlegl_buffer_attach(void *data, struct wl_resource *buffer_res, GLuint program,
		    GLuint textures[])
{
	struct wlegl_buffer *buffer;
	EGLDisplay display;
	EGLImageKHR image;

	buffer = wl_resource_get_user_data(buffer_res);

	display = eglGetCurrentDisplay();

	image = eglCreateImageKHR(display, EGL_NO_CONTEXT,
				  EGL_NATIVE_BUFFER_ANDROID,
				  &buffer->native, NULL);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures[0]);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
	eglDestroyImageKHR(display, image);
}

const struct wlb_buffer_type wlegl_buffer_type = {
	wlegl_buffer_is_type,
	wlegl_buffer_get_size,
	NULL, NULL,
"#extension GL_OES_EGL_image_external : require\n"
"uniform samplerExternalOES tex;\n"
"lowp vec4 wlb_get_fragment_color(mediump vec2 coords)\n"
"{\n"
"	return texture2D(tex, coords)\n;"
"}\n", 1,
	NULL, wlegl_buffer_attach, NULL
};
