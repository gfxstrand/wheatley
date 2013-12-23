#ifndef __WHEATLEY_WLEGL_H__
#define __WHEATLEY_WLEGL_H__

#include <libwlb.h>

struct wlegl;

struct wlegl *
wlegl_create(struct wlb_compositor *compositor);
void
wlegl_destroy(struct wlegl *wlegl);

#endif /* ! defined __WHEATLEY_WLEGL_H__ */
