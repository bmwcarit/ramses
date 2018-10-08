/* 
 * Copyright (C) 2013 DENSO CORPORATION
 * Copyright (c) 2013 BMW Car IT GmbH
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef IVI_APPLICATION_CLIENT_PROTOCOL_H
#define IVI_APPLICATION_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct ivi_surface;
struct ivi_application;

extern const struct wl_interface ivi_surface_interface;
extern const struct wl_interface ivi_application_interface;

/**
 * ivi_surface - application interface to surface in ivi compositor
 * @configure: suggest resize
 *
 * 
 */
struct ivi_surface_listener {
	/**
	 * configure - suggest resize
	 * @width: (none)
	 * @height: (none)
	 *
	 * The configure event asks the client to resize its surface.
	 *
	 * The size is a hint, in the sense that the client is free to
	 * ignore it if it doesn't resize, pick a smaller size (to satisfy
	 * aspect ratio or resize in steps of NxM pixels).
	 *
	 * The client is free to dismiss all but the last configure event
	 * it received.
	 *
	 * The width and height arguments specify the size of the window in
	 * surface local coordinates.
	 */
	void (*configure)(void *data,
			  struct ivi_surface *ivi_surface,
			  int32_t width,
			  int32_t height);
};

static inline int
ivi_surface_add_listener(struct ivi_surface *ivi_surface,
			 const struct ivi_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) ivi_surface,
				     (void (**)(void)) listener, data);
}

#define IVI_SURFACE_DESTROY	0

static inline void
ivi_surface_set_user_data(struct ivi_surface *ivi_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_surface, user_data);
}

static inline void *
ivi_surface_get_user_data(struct ivi_surface *ivi_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_surface);
}

static inline void
ivi_surface_destroy(struct ivi_surface *ivi_surface)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_surface,
			 IVI_SURFACE_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) ivi_surface);
}

#ifndef IVI_APPLICATION_ERROR_ENUM
#define IVI_APPLICATION_ERROR_ENUM
enum ivi_application_error {
	IVI_APPLICATION_ERROR_ROLE = 0,
	IVI_APPLICATION_ERROR_IVI_ID = 1,
};
#endif /* IVI_APPLICATION_ERROR_ENUM */

#define IVI_APPLICATION_SURFACE_CREATE	0

static inline void
ivi_application_set_user_data(struct ivi_application *ivi_application, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_application, user_data);
}

static inline void *
ivi_application_get_user_data(struct ivi_application *ivi_application)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_application);
}

static inline void
ivi_application_destroy(struct ivi_application *ivi_application)
{
	wl_proxy_destroy((struct wl_proxy *) ivi_application);
}

static inline struct ivi_surface *
ivi_application_surface_create(struct ivi_application *ivi_application, uint32_t ivi_id, struct wl_surface *surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) ivi_application,
			 IVI_APPLICATION_SURFACE_CREATE, &ivi_surface_interface, ivi_id, surface, NULL);

	return (struct ivi_surface *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
