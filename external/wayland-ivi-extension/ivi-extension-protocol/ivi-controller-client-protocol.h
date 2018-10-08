/* 
 * Copyright (C) 2013 DENSO CORPORATION
 * Copyright (c) 2013 BMW Car IT GmbH
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef IVI_CONTROLLER_CLIENT_PROTOCOL_H
#define IVI_CONTROLLER_CLIENT_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

struct wl_client;
struct wl_resource;

struct ivi_controller_surface;
struct ivi_controller_layer;
struct ivi_controller_screen;
struct ivi_controller;

extern const struct wl_interface ivi_controller_surface_interface;
extern const struct wl_interface ivi_controller_layer_interface;
extern const struct wl_interface ivi_controller_screen_interface;
extern const struct wl_interface ivi_controller_interface;

#ifndef IVI_CONTROLLER_SURFACE_ORIENTATION_ENUM
#define IVI_CONTROLLER_SURFACE_ORIENTATION_ENUM
/**
 * ivi_controller_surface_orientation - orientation presets in degrees
 * @IVI_CONTROLLER_SURFACE_ORIENTATION_0_DEGREES: not rotated
 * @IVI_CONTROLLER_SURFACE_ORIENTATION_90_DEGREES: rotated 90 degrees
 *	clockwise
 * @IVI_CONTROLLER_SURFACE_ORIENTATION_180_DEGREES: rotated 180 degrees
 *	clockwise
 * @IVI_CONTROLLER_SURFACE_ORIENTATION_270_DEGREES: rotated 270 degrees
 *	clockwise
 *
 * The surfaces in ivi controller can be rotated in 90 degrees steps.
 * This enum defines all valid orientations for surfaces.
 */
enum ivi_controller_surface_orientation {
	IVI_CONTROLLER_SURFACE_ORIENTATION_0_DEGREES = 0,
	IVI_CONTROLLER_SURFACE_ORIENTATION_90_DEGREES = 1,
	IVI_CONTROLLER_SURFACE_ORIENTATION_180_DEGREES = 2,
	IVI_CONTROLLER_SURFACE_ORIENTATION_270_DEGREES = 3,
};
#endif /* IVI_CONTROLLER_SURFACE_ORIENTATION_ENUM */

#ifndef IVI_CONTROLLER_SURFACE_PIXELFORMAT_ENUM
#define IVI_CONTROLLER_SURFACE_PIXELFORMAT_ENUM
/**
 * ivi_controller_surface_pixelformat - pixel format values
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_R_8: 8 bit luminance surface
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGB_888: 24 bit rgb surface
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_8888: 24 bit rgb surface with
 *	8 bit alpha
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGB_565: 16 bit rgb surface
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_5551: 16 bit rgb surface with
 *	binary mask
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_6661: 18 bit rgb surface with
 *	binary mask
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_4444: 12 bit rgb surface with
 *	4 bit alpha
 * @IVI_CONTROLLER_SURFACE_PIXELFORMAT_UNKNOWN: unknown
 *
 * Applications can provide buffers as surface content with differernt
 * buffer properties. This enum defines all supported buffer
 * configurations.
 */
enum ivi_controller_surface_pixelformat {
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_R_8 = 0,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGB_888 = 1,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_8888 = 2,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGB_565 = 3,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_5551 = 4,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_6661 = 5,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_RGBA_4444 = 6,
	IVI_CONTROLLER_SURFACE_PIXELFORMAT_UNKNOWN = 7,
};
#endif /* IVI_CONTROLLER_SURFACE_PIXELFORMAT_ENUM */

#ifndef IVI_CONTROLLER_SURFACE_CONTENT_STATE_ENUM
#define IVI_CONTROLLER_SURFACE_CONTENT_STATE_ENUM
/**
 * ivi_controller_surface_content_state - all possible states of content
 *	for a surface
 * @IVI_CONTROLLER_SURFACE_CONTENT_STATE_CONTENT_AVAILABLE: application
 *	provided wl_surface for this surface
 * @IVI_CONTROLLER_SURFACE_CONTENT_STATE_CONTENT_REMOVED: wl_surface was
 *	removed for this surface
 *
 * This enum defines all possible content states of a surface. This is
 * required, since surfaces in ivi compositor can exist without
 * applications providing content for them.
 */
enum ivi_controller_surface_content_state {
	IVI_CONTROLLER_SURFACE_CONTENT_STATE_CONTENT_AVAILABLE = 1,
	IVI_CONTROLLER_SURFACE_CONTENT_STATE_CONTENT_REMOVED = 2,
};
#endif /* IVI_CONTROLLER_SURFACE_CONTENT_STATE_ENUM */

/**
 * ivi_controller_surface - controller interface to surface in ivi
 *	compositor
 * @visibility: the visibility of the surface in ivi compositor has
 *	changed
 * @opacity: the opacity of surface in ivi compositor has changed
 * @source_rectangle: the source rectangle of surface in ivi compositor
 *	has changed
 * @destination_rectangle: the destination rectangle of surface in ivi
 *	compositor has changed
 * @configuration: the configuration of surface in ivi compositor has
 *	changed
 * @orientation: the orientation of surface in ivi compositor has changed
 * @pixelformat: pixelformat for surface in ivi compositor has changed
 * @layer: surface in ivi compositor was added to a layer
 * @stats: receive updated statistics for surface in ivi compositor
 * @destroyed: ivi_controller_surface was destroyed
 * @content: content state for surface has changed
 *
 * 
 */
struct ivi_controller_surface_listener {
	/**
	 * visibility - the visibility of the surface in ivi compositor
	 *	has changed
	 * @visibility: (none)
	 *
	 * The new visibility state is provided in argument visibility.
	 * If visibility is 0, the surface has become invisible. If
	 * visibility is not 0, the surface has become visible.
	 */
	void (*visibility)(void *data,
			   struct ivi_controller_surface *ivi_controller_surface,
			   int32_t visibility);
	/**
	 * opacity - the opacity of surface in ivi compositor has changed
	 * @opacity: (none)
	 *
	 * The new opacity state is provided in argument opacity. The
	 * valid range for opactiy is 0.0 (fully transparent) to 1.0 (fully
	 * opaque).
	 */
	void (*opacity)(void *data,
			struct ivi_controller_surface *ivi_controller_surface,
			wl_fixed_t opacity);
	/**
	 * source_rectangle - the source rectangle of surface in ivi
	 *	compositor has changed
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The scanout region of the surface content has changed. The new
	 * values for source rectangle are provided by x: new horizontal
	 * start position of scanout area within the surface y: new
	 * vertical start position of scanout area within the surface
	 * width: new width of scanout area within the surface height: new
	 * height of scanout area within the surface
	 */
	void (*source_rectangle)(void *data,
				 struct ivi_controller_surface *ivi_controller_surface,
				 int32_t x,
				 int32_t y,
				 int32_t width,
				 int32_t height);
	/**
	 * destination_rectangle - the destination rectangle of surface
	 *	in ivi compositor has changed
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The new values for source rectangle are provided by x: new
	 * horizontal start position of surface within the layer y: new
	 * vertical start position of surface within the layer width : new
	 * width of surface within the layer height: new height of surface
	 * within the layer
	 */
	void (*destination_rectangle)(void *data,
				      struct ivi_controller_surface *ivi_controller_surface,
				      int32_t x,
				      int32_t y,
				      int32_t width,
				      int32_t height);
	/**
	 * configuration - the configuration of surface in ivi compositor
	 *	has changed
	 * @width: (none)
	 * @height: (none)
	 *
	 * The client providing content for this surface was requested to
	 * resize the buffer provided as surface content. The requested
	 * buffer size is provided by arguments width and height.
	 */
	void (*configuration)(void *data,
			      struct ivi_controller_surface *ivi_controller_surface,
			      int32_t width,
			      int32_t height);
	/**
	 * orientation - the orientation of surface in ivi compositor has
	 *	changed
	 * @orientation: (none)
	 *
	 * The new orientation status is provided by argument
	 * orientation.
	 */
	void (*orientation)(void *data,
			    struct ivi_controller_surface *ivi_controller_surface,
			    int32_t orientation);
	/**
	 * pixelformat - pixelformat for surface in ivi compositor has
	 *	changed
	 * @pixelformat: (none)
	 *
	 * When client attach buffers as surface content, these buffers
	 * have a pixelformat configuration. If the pixelformat of a newly
	 * attached buffer is different from the previous buffer
	 * configuration, this event is raised. This is also done, when the
	 * first buffer is provided by application.
	 */
	void (*pixelformat)(void *data,
			    struct ivi_controller_surface *ivi_controller_surface,
			    int32_t pixelformat);
	/**
	 * layer - surface in ivi compositor was added to a layer
	 * @layer: (none)
	 *
	 * This surface was added to the render order of the layer
	 * defined by argument layer. This is essential for a surface to
	 * become visible on screen, since ivi compositors will only render
	 * layers (or more precise all surfaces in the render order of a
	 * layer).
	 */
	void (*layer)(void *data,
		      struct ivi_controller_surface *ivi_controller_surface,
		      struct ivi_controller_layer *layer);
	/**
	 * stats - receive updated statistics for surface in ivi
	 *	compositor
	 * @redraw_count: (none)
	 * @frame_count: (none)
	 * @update_count: (none)
	 * @pid: (none)
	 * @process_name: (none)
	 *
	 * The information contained in this event is essential for
	 * monitoring, debugging, logging and tracing support in IVI
	 * systems.
	 */
	void (*stats)(void *data,
		      struct ivi_controller_surface *ivi_controller_surface,
		      uint32_t redraw_count,
		      uint32_t frame_count,
		      uint32_t update_count,
		      uint32_t pid,
		      const char *process_name);
	/**
	 * destroyed - ivi_controller_surface was destroyed
	 *
	 * 
	 */
	void (*destroyed)(void *data,
			  struct ivi_controller_surface *ivi_controller_surface);
	/**
	 * content - content state for surface has changed
	 * @content_state: (none)
	 *
	 * Surfaces in ivi compositor can exist without any application
	 * or controller referencing it. All surfaces initially have no
	 * content. This event indicates when content state has changed.
	 * All possible content states are defined in enum content_state.
	 */
	void (*content)(void *data,
			struct ivi_controller_surface *ivi_controller_surface,
			int32_t content_state);
};

static inline int
ivi_controller_surface_add_listener(struct ivi_controller_surface *ivi_controller_surface,
				    const struct ivi_controller_surface_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) ivi_controller_surface,
				     (void (**)(void)) listener, data);
}

#define IVI_CONTROLLER_SURFACE_SET_VISIBILITY	0
#define IVI_CONTROLLER_SURFACE_SET_OPACITY	1
#define IVI_CONTROLLER_SURFACE_SET_SOURCE_RECTANGLE	2
#define IVI_CONTROLLER_SURFACE_SET_DESTINATION_RECTANGLE	3
#define IVI_CONTROLLER_SURFACE_SET_CONFIGURATION	4
#define IVI_CONTROLLER_SURFACE_SET_ORIENTATION	5
#define IVI_CONTROLLER_SURFACE_SCREENSHOT	6
#define IVI_CONTROLLER_SURFACE_SEND_STATS	7
#define IVI_CONTROLLER_SURFACE_DESTROY	8

static inline void
ivi_controller_surface_set_user_data(struct ivi_controller_surface *ivi_controller_surface, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_controller_surface, user_data);
}

static inline void *
ivi_controller_surface_get_user_data(struct ivi_controller_surface *ivi_controller_surface)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_controller_surface);
}

static inline void
ivi_controller_surface_set_visibility(struct ivi_controller_surface *ivi_controller_surface, uint32_t visibility)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_VISIBILITY, visibility);
}

static inline void
ivi_controller_surface_set_opacity(struct ivi_controller_surface *ivi_controller_surface, wl_fixed_t opacity)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_OPACITY, opacity);
}

static inline void
ivi_controller_surface_set_source_rectangle(struct ivi_controller_surface *ivi_controller_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_SOURCE_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_surface_set_destination_rectangle(struct ivi_controller_surface *ivi_controller_surface, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_DESTINATION_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_surface_set_configuration(struct ivi_controller_surface *ivi_controller_surface, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_CONFIGURATION, width, height);
}

static inline void
ivi_controller_surface_set_orientation(struct ivi_controller_surface *ivi_controller_surface, int32_t orientation)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SET_ORIENTATION, orientation);
}

static inline void
ivi_controller_surface_screenshot(struct ivi_controller_surface *ivi_controller_surface, const char *filename)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SCREENSHOT, filename);
}

static inline void
ivi_controller_surface_send_stats(struct ivi_controller_surface *ivi_controller_surface)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_SEND_STATS);
}

static inline void
ivi_controller_surface_destroy(struct ivi_controller_surface *ivi_controller_surface, int32_t destroy_scene_object)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_surface,
			 IVI_CONTROLLER_SURFACE_DESTROY, destroy_scene_object);

	wl_proxy_destroy((struct wl_proxy *) ivi_controller_surface);
}

/**
 * ivi_controller_layer - controller interface to layer in ivi compositor
 * @visibility: the visibility of the layer in ivi compositor has changed
 * @opacity: the opacity of layer in ivi compositor has changed
 * @source_rectangle: the source rectangle of layer in ivi compositor has
 *	changed
 * @destination_rectangle: the destination rectangle of layer in ivi
 *	compositor has changed
 * @configuration: the configuration of layer in ivi compositor has
 *	changed
 * @orientation: the orientation of layer in ivi compositor has changed
 * @screen: layer in ivi compositor was added to a screen
 * @destroyed: destroyed layer event
 *
 * 
 */
struct ivi_controller_layer_listener {
	/**
	 * visibility - the visibility of the layer in ivi compositor has
	 *	changed
	 * @visibility: (none)
	 *
	 * The new visibility state is provided in argument visibility.
	 * If visibility is 0, the layer has become invisible. If
	 * visibility is not 0, the layer has become visible.
	 */
	void (*visibility)(void *data,
			   struct ivi_controller_layer *ivi_controller_layer,
			   int32_t visibility);
	/**
	 * opacity - the opacity of layer in ivi compositor has changed
	 * @opacity: (none)
	 *
	 * The new opacity state is provided in argument opacity. The
	 * valid range for opactiy is 0.0 (fully transparent) to 1.0 (fully
	 * opaque).
	 */
	void (*opacity)(void *data,
			struct ivi_controller_layer *ivi_controller_layer,
			wl_fixed_t opacity);
	/**
	 * source_rectangle - the source rectangle of layer in ivi
	 *	compositor has changed
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The scanout region of the layer content has changed. The new
	 * values for source rectangle are provided by x: new horizontal
	 * start position of scanout area within the layer y: new vertical
	 * start position of scanout area within the layer width: new width
	 * of scanout area within the layer height: new height of scanout
	 * area within the layer
	 */
	void (*source_rectangle)(void *data,
				 struct ivi_controller_layer *ivi_controller_layer,
				 int32_t x,
				 int32_t y,
				 int32_t width,
				 int32_t height);
	/**
	 * destination_rectangle - the destination rectangle of layer in
	 *	ivi compositor has changed
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The new values for source rectangle are provided by x: new
	 * horizontal start position of layer within the screen y: new
	 * vertical start position of layer within the screen width : new
	 * width of layer within the screen height: new height of layer
	 * within the screen
	 */
	void (*destination_rectangle)(void *data,
				      struct ivi_controller_layer *ivi_controller_layer,
				      int32_t x,
				      int32_t y,
				      int32_t width,
				      int32_t height);
	/**
	 * configuration - the configuration of layer in ivi compositor
	 *	has changed
	 * @width: (none)
	 * @height: (none)
	 *
	 * The layer was resized. The new layer size is provided by
	 * arguments width and height.
	 */
	void (*configuration)(void *data,
			      struct ivi_controller_layer *ivi_controller_layer,
			      int32_t width,
			      int32_t height);
	/**
	 * orientation - the orientation of layer in ivi compositor has
	 *	changed
	 * @orientation: (none)
	 *
	 * The new orientation status is provided by argument
	 * orientation.
	 */
	void (*orientation)(void *data,
			    struct ivi_controller_layer *ivi_controller_layer,
			    int32_t orientation);
	/**
	 * screen - layer in ivi compositor was added to a screen
	 * @screen: (none)
	 *
	 * This layer was added to the render order of the screen defined
	 * by argument screen. This is essential for a layer to become
	 * visible on screen, since ivi compositors will only render
	 * screens (or more precise all layers in the render order of a
	 * screen).
	 */
	void (*screen)(void *data,
		       struct ivi_controller_layer *ivi_controller_layer,
		       struct wl_output *screen);
	/**
	 * destroyed - destroyed layer event
	 *
	 * 
	 */
	void (*destroyed)(void *data,
			  struct ivi_controller_layer *ivi_controller_layer);
};

static inline int
ivi_controller_layer_add_listener(struct ivi_controller_layer *ivi_controller_layer,
				  const struct ivi_controller_layer_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) ivi_controller_layer,
				     (void (**)(void)) listener, data);
}

#define IVI_CONTROLLER_LAYER_SET_VISIBILITY	0
#define IVI_CONTROLLER_LAYER_SET_OPACITY	1
#define IVI_CONTROLLER_LAYER_SET_SOURCE_RECTANGLE	2
#define IVI_CONTROLLER_LAYER_SET_DESTINATION_RECTANGLE	3
#define IVI_CONTROLLER_LAYER_SET_CONFIGURATION	4
#define IVI_CONTROLLER_LAYER_SET_ORIENTATION	5
#define IVI_CONTROLLER_LAYER_SCREENSHOT	6
#define IVI_CONTROLLER_LAYER_CLEAR_SURFACES	7
#define IVI_CONTROLLER_LAYER_ADD_SURFACE	8
#define IVI_CONTROLLER_LAYER_REMOVE_SURFACE	9
#define IVI_CONTROLLER_LAYER_SET_RENDER_ORDER	10
#define IVI_CONTROLLER_LAYER_DESTROY	11

static inline void
ivi_controller_layer_set_user_data(struct ivi_controller_layer *ivi_controller_layer, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_controller_layer, user_data);
}

static inline void *
ivi_controller_layer_get_user_data(struct ivi_controller_layer *ivi_controller_layer)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_controller_layer);
}

static inline void
ivi_controller_layer_set_visibility(struct ivi_controller_layer *ivi_controller_layer, uint32_t visibility)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_VISIBILITY, visibility);
}

static inline void
ivi_controller_layer_set_opacity(struct ivi_controller_layer *ivi_controller_layer, wl_fixed_t opacity)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_OPACITY, opacity);
}

static inline void
ivi_controller_layer_set_source_rectangle(struct ivi_controller_layer *ivi_controller_layer, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_SOURCE_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_layer_set_destination_rectangle(struct ivi_controller_layer *ivi_controller_layer, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_DESTINATION_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_layer_set_configuration(struct ivi_controller_layer *ivi_controller_layer, int32_t width, int32_t height)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_CONFIGURATION, width, height);
}

static inline void
ivi_controller_layer_set_orientation(struct ivi_controller_layer *ivi_controller_layer, int32_t orientation)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_ORIENTATION, orientation);
}

static inline void
ivi_controller_layer_screenshot(struct ivi_controller_layer *ivi_controller_layer, const char *filename)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SCREENSHOT, filename);
}

static inline void
ivi_controller_layer_clear_surfaces(struct ivi_controller_layer *ivi_controller_layer)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_CLEAR_SURFACES);
}

static inline void
ivi_controller_layer_add_surface(struct ivi_controller_layer *ivi_controller_layer, struct ivi_controller_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_ADD_SURFACE, surface);
}

static inline void
ivi_controller_layer_remove_surface(struct ivi_controller_layer *ivi_controller_layer, struct ivi_controller_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_REMOVE_SURFACE, surface);
}

static inline void
ivi_controller_layer_set_render_order(struct ivi_controller_layer *ivi_controller_layer, struct wl_array *id_surfaces)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_SET_RENDER_ORDER, id_surfaces);
}

static inline void
ivi_controller_layer_destroy(struct ivi_controller_layer *ivi_controller_layer, int32_t destroy_scene_object)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_layer,
			 IVI_CONTROLLER_LAYER_DESTROY, destroy_scene_object);

	wl_proxy_destroy((struct wl_proxy *) ivi_controller_layer);
}

#define IVI_CONTROLLER_SCREEN_DESTROY	0
#define IVI_CONTROLLER_SCREEN_CLEAR	1
#define IVI_CONTROLLER_SCREEN_ADD_LAYER	2
#define IVI_CONTROLLER_SCREEN_SCREENSHOT	3
#define IVI_CONTROLLER_SCREEN_SET_RENDER_ORDER	4

static inline void
ivi_controller_screen_set_user_data(struct ivi_controller_screen *ivi_controller_screen, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_controller_screen, user_data);
}

static inline void *
ivi_controller_screen_get_user_data(struct ivi_controller_screen *ivi_controller_screen)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_controller_screen);
}

static inline void
ivi_controller_screen_destroy(struct ivi_controller_screen *ivi_controller_screen)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_screen,
			 IVI_CONTROLLER_SCREEN_DESTROY);

	wl_proxy_destroy((struct wl_proxy *) ivi_controller_screen);
}

static inline void
ivi_controller_screen_clear(struct ivi_controller_screen *ivi_controller_screen)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_screen,
			 IVI_CONTROLLER_SCREEN_CLEAR);
}

static inline void
ivi_controller_screen_add_layer(struct ivi_controller_screen *ivi_controller_screen, struct ivi_controller_layer *layer)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_screen,
			 IVI_CONTROLLER_SCREEN_ADD_LAYER, layer);
}

static inline void
ivi_controller_screen_screenshot(struct ivi_controller_screen *ivi_controller_screen, const char *filename)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_screen,
			 IVI_CONTROLLER_SCREEN_SCREENSHOT, filename);
}

static inline void
ivi_controller_screen_set_render_order(struct ivi_controller_screen *ivi_controller_screen, struct wl_array *id_layers)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller_screen,
			 IVI_CONTROLLER_SCREEN_SET_RENDER_ORDER, id_layers);
}

#ifndef IVI_CONTROLLER_OBJECT_TYPE_ENUM
#define IVI_CONTROLLER_OBJECT_TYPE_ENUM
/**
 * ivi_controller_object_type - available object types in ivi compositor
 *	scene
 * @IVI_CONTROLLER_OBJECT_TYPE_SURFACE: surface object type
 * @IVI_CONTROLLER_OBJECT_TYPE_LAYER: layer object type
 * @IVI_CONTROLLER_OBJECT_TYPE_SCREEN: screen object type
 *
 * This enum defines all scene object available in ivi compositor.
 */
enum ivi_controller_object_type {
	IVI_CONTROLLER_OBJECT_TYPE_SURFACE = 1,
	IVI_CONTROLLER_OBJECT_TYPE_LAYER = 2,
	IVI_CONTROLLER_OBJECT_TYPE_SCREEN = 3,
};
#endif /* IVI_CONTROLLER_OBJECT_TYPE_ENUM */

#ifndef IVI_CONTROLLER_ERROR_CODE_ENUM
#define IVI_CONTROLLER_ERROR_CODE_ENUM
/**
 * ivi_controller_error_code - possible error codes returned in error
 *	event
 * @IVI_CONTROLLER_ERROR_CODE_UNKNOWN_ERROR: unknown error encountered
 * @IVI_CONTROLLER_ERROR_CODE_FILE_ERROR: file i/o error encountered
 *
 * These error codes define all possible error codes returned by ivi
 * compositor on server-side errors.
 */
enum ivi_controller_error_code {
	IVI_CONTROLLER_ERROR_CODE_UNKNOWN_ERROR = 1,
	IVI_CONTROLLER_ERROR_CODE_FILE_ERROR = 2,
};
#endif /* IVI_CONTROLLER_ERROR_CODE_ENUM */

/**
 * ivi_controller - interface for ivi controllers to use ivi compositor
 *	features
 * @screen: new screen is available
 * @layer: new layer is available
 * @surface: new surface is available
 * @error: server-side error detected
 *
 * 
 */
struct ivi_controller_listener {
	/**
	 * screen - new screen is available
	 * @id_screen: (none)
	 * @screen: (none)
	 *
	 * A new screen is announced to the controller. This is typically
	 * the case in two cases: 1. controller was just started, ivi
	 * compositor announces existing screen 2. a new screen was added
	 * to the system at runtime
	 */
	void (*screen)(void *data,
		       struct ivi_controller *ivi_controller,
		       uint32_t id_screen,
		       struct ivi_controller_screen *screen);
	/**
	 * layer - new layer is available
	 * @id_layer: (none)
	 *
	 * A new layer is announced to the controller.
	 */
	void (*layer)(void *data,
		      struct ivi_controller *ivi_controller,
		      uint32_t id_layer);
	/**
	 * surface - new surface is available
	 * @id_surface: (none)
	 *
	 * A new surface is announced to the controller.
	 */
	void (*surface)(void *data,
			struct ivi_controller *ivi_controller,
			uint32_t id_surface);
	/**
	 * error - server-side error detected
	 * @object_id: (none)
	 * @object_type: (none)
	 * @error_code: (none)
	 * @error_text: (none)
	 *
	 * The ivi compositor encountered error while processing a
	 * request by this controller. The error is defined by argument
	 * error_code and optional error_text. Additionally the object type
	 * and id is contained in the error event to provide some detailes
	 * to handle the error. If the controller requires to associate
	 * this error event to a request, it can 1. send request 2. force
	 * display roundtrip 3. check, if error event was received but this
	 * restricts the controller to have only one open request at a
	 * time.
	 */
	void (*error)(void *data,
		      struct ivi_controller *ivi_controller,
		      int32_t object_id,
		      int32_t object_type,
		      int32_t error_code,
		      const char *error_text);
};

static inline int
ivi_controller_add_listener(struct ivi_controller *ivi_controller,
			    const struct ivi_controller_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) ivi_controller,
				     (void (**)(void)) listener, data);
}

#define IVI_CONTROLLER_COMMIT_CHANGES	0
#define IVI_CONTROLLER_LAYER_CREATE	1
#define IVI_CONTROLLER_SURFACE_CREATE	2

static inline void
ivi_controller_set_user_data(struct ivi_controller *ivi_controller, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ivi_controller, user_data);
}

static inline void *
ivi_controller_get_user_data(struct ivi_controller *ivi_controller)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ivi_controller);
}

static inline void
ivi_controller_destroy(struct ivi_controller *ivi_controller)
{
	wl_proxy_destroy((struct wl_proxy *) ivi_controller);
}

static inline void
ivi_controller_commit_changes(struct ivi_controller *ivi_controller)
{
	wl_proxy_marshal((struct wl_proxy *) ivi_controller,
			 IVI_CONTROLLER_COMMIT_CHANGES);
}

static inline struct ivi_controller_layer *
ivi_controller_layer_create(struct ivi_controller *ivi_controller, uint32_t id_layer, int32_t width, int32_t height)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) ivi_controller,
			 IVI_CONTROLLER_LAYER_CREATE, &ivi_controller_layer_interface, id_layer, width, height, NULL);

	return (struct ivi_controller_layer *) id;
}

static inline struct ivi_controller_surface *
ivi_controller_surface_create(struct ivi_controller *ivi_controller, uint32_t id_surface)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_constructor((struct wl_proxy *) ivi_controller,
			 IVI_CONTROLLER_SURFACE_CREATE, &ivi_controller_surface_interface, id_surface, NULL);

	return (struct ivi_controller_surface *) id;
}

#ifdef  __cplusplus
}
#endif

#endif
