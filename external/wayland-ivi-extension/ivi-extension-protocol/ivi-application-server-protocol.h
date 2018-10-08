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

#ifndef IVI_APPLICATION_SERVER_PROTOCOL_H
#define IVI_APPLICATION_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

struct wl_client;
struct wl_resource;

struct ivi_surface;
struct ivi_application;

extern const struct wl_interface ivi_surface_interface;
extern const struct wl_interface ivi_application_interface;

/**
 * ivi_surface - application interface to surface in ivi compositor
 * @destroy: destroy ivi_surface
 *
 * 
 */
struct ivi_surface_interface {
	/**
	 * destroy - destroy ivi_surface
	 *
	 * This removes link from ivi_id to wl_surface and destroys
	 * ivi_surface. The ID, ivi_id, is free and can be used for
	 * surface_create again.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
};

#define IVI_SURFACE_CONFIGURE	0

#define IVI_SURFACE_CONFIGURE_SINCE_VERSION	1

static inline void
ivi_surface_send_configure(struct wl_resource *resource_, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_SURFACE_CONFIGURE, width, height);
}

#ifndef IVI_APPLICATION_ERROR_ENUM
#define IVI_APPLICATION_ERROR_ENUM
enum ivi_application_error {
	IVI_APPLICATION_ERROR_ROLE = 0,
	IVI_APPLICATION_ERROR_IVI_ID = 1,
};
#endif /* IVI_APPLICATION_ERROR_ENUM */

/**
 * ivi_application - create ivi-style surfaces
 * @surface_create: create ivi_surface with numeric ID in ivi compositor
 *
 * This interface is exposed as a global singleton. This interface is
 * implemented by servers that provide IVI-style user interfaces. It allows
 * clients to associate a ivi_surface with wl_surface.
 */
struct ivi_application_interface {
	/**
	 * surface_create - create ivi_surface with numeric ID in ivi
	 *	compositor
	 * @ivi_id: (none)
	 * @surface: (none)
	 * @id: (none)
	 *
	 * This request gives the wl_surface the role of an IVI Surface.
	 * Creating more than one ivi_surface for a wl_surface is not
	 * allowed. Note, that this still allows the following example:
	 *
	 * 1. create a wl_surface 2. create ivi_surface for the wl_surface
	 * 3. destroy the ivi_surface 4. create ivi_surface for the
	 * wl_surface (with the same or another ivi_id as before)
	 *
	 * surface_create will create a interface:ivi_surface with numeric
	 * ID; ivi_id in ivi compositor. These ivi_ids are defined as
	 * unique in the system to identify it inside of ivi compositor.
	 * The ivi compositor implements business logic how to set
	 * properties of the surface with ivi_id according to status of the
	 * system. E.g. a unique ID for Car Navigation application is used
	 * for implementing special logic of the application about where it
	 * shall be located. The server regards following cases as protocol
	 * errors and disconnects the client. - wl_surface already has an
	 * nother role. - ivi_id is already assigned to an another
	 * wl_surface.
	 *
	 * If client destroys ivi_surface or wl_surface which is assigne to
	 * the ivi_surface, ivi_id which is assigned to the ivi_surface is
	 * free for reuse.
	 */
	void (*surface_create)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t ivi_id,
			       struct wl_resource *surface,
			       uint32_t id);
};


#ifdef  __cplusplus
}
#endif

#endif
