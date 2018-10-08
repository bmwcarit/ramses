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

#ifndef IVI_CONTROLLER_SERVER_PROTOCOL_H
#define IVI_CONTROLLER_SERVER_PROTOCOL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "wayland-server.h"

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
 * @set_visibility: set the visibility of a surface in ivi compositor
 * @set_opacity: set the opacity of a surface in ivi compositor
 * @set_source_rectangle: set the scanout area of a surface in ivi
 *	compositor
 * @set_destination_rectangle: Set the destination area of a surface
 *	within a layer
 * @set_configuration: request new buffer size for application content
 * @set_orientation: set the orientation of a surface in ivi compositor
 * @screenshot: take screenshot of surface
 * @send_stats: request statistics for surface in ivi compositor
 * @destroy: destroy ivi_controller_surface
 *
 * 
 */
struct ivi_controller_surface_interface {
	/**
	 * set_visibility - set the visibility of a surface in ivi
	 *	compositor
	 * @visibility: (none)
	 *
	 * If visibility argument is 0, the surface in the ivi compositor
	 * is set to invisible. If visibility argument is not 0, the
	 * surface in the ivi compositor is set to visible.
	 */
	void (*set_visibility)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t visibility);
	/**
	 * set_opacity - set the opacity of a surface in ivi compositor
	 * @opacity: (none)
	 *
	 * The valid range for opacity is 0.0 (fully transparent) to 1.0
	 * (fully opaque).
	 */
	void (*set_opacity)(struct wl_client *client,
			    struct wl_resource *resource,
			    wl_fixed_t opacity);
	/**
	 * set_source_rectangle - set the scanout area of a surface in
	 *	ivi compositor
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The source rectangle defines the part of the surface content,
	 * that is used for compositing the surface. It can be used, if
	 * valid content of the surface is smaller than the surface.
	 * Effectively it can be used to zoom the content of the surface.
	 * x: horizontal start position of scanout area within the surface
	 * y: vertical start position of scanout area within the surface
	 * width: width of scanout area within the surface height: height
	 * of scanout area within the surface
	 */
	void (*set_source_rectangle)(struct wl_client *client,
				     struct wl_resource *resource,
				     int32_t x,
				     int32_t y,
				     int32_t width,
				     int32_t height);
	/**
	 * set_destination_rectangle - Set the destination area of a
	 *	surface within a layer
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The destination rectangle defines the position and size of a
	 * surface on a layer. The surface will be scaled to this rectangle
	 * for rendering. x: horizontal start position of surface within
	 * the layer y: vertical start position of surface within the layer
	 * width : width of surface within the layer height: height of
	 * surface within the layer
	 */
	void (*set_destination_rectangle)(struct wl_client *client,
					  struct wl_resource *resource,
					  int32_t x,
					  int32_t y,
					  int32_t width,
					  int32_t height);
	/**
	 * set_configuration - request new buffer size for application
	 *	content
	 * @width: (none)
	 * @height: (none)
	 *
	 * Request the client providing content for this surface, to
	 * resize of the buffers provided as surface content.
	 */
	void (*set_configuration)(struct wl_client *client,
				  struct wl_resource *resource,
				  int32_t width,
				  int32_t height);
	/**
	 * set_orientation - set the orientation of a surface in ivi
	 *	compositor
	 * @orientation: (none)
	 *
	 * The orientation of a surface in ivi compositor can be rotated
	 * in 90 degree steps, as defined in orientation enum.
	 */
	void (*set_orientation)(struct wl_client *client,
				struct wl_resource *resource,
				int32_t orientation);
	/**
	 * screenshot - take screenshot of surface
	 * @filename: (none)
	 *
	 * Store a screenshot of the surface content in the file provided
	 * by argument filename.
	 */
	void (*screenshot)(struct wl_client *client,
			   struct wl_resource *resource,
			   const char *filename);
	/**
	 * send_stats - request statistics for surface in ivi compositor
	 *
	 * These stats contain information required for monitoring,
	 * debugging, logging and tracing.
	 */
	void (*send_stats)(struct wl_client *client,
			   struct wl_resource *resource);
	/**
	 * destroy - destroy ivi_controller_surface
	 * @destroy_scene_object: (none)
	 *
	 * Request to destroy the ivi_controller_surface. If argument
	 * destroy_scene_object id not 0, the surface will be destroyed in
	 * ivi compositor. If argument is 0, only the proxy object is
	 * destroyed.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource,
			int32_t destroy_scene_object);
};

#define IVI_CONTROLLER_SURFACE_VISIBILITY	0
#define IVI_CONTROLLER_SURFACE_OPACITY	1
#define IVI_CONTROLLER_SURFACE_SOURCE_RECTANGLE	2
#define IVI_CONTROLLER_SURFACE_DESTINATION_RECTANGLE	3
#define IVI_CONTROLLER_SURFACE_CONFIGURATION	4
#define IVI_CONTROLLER_SURFACE_ORIENTATION	5
#define IVI_CONTROLLER_SURFACE_PIXELFORMAT	6
#define IVI_CONTROLLER_SURFACE_LAYER	7
#define IVI_CONTROLLER_SURFACE_STATS	8
#define IVI_CONTROLLER_SURFACE_DESTROYED	9
#define IVI_CONTROLLER_SURFACE_CONTENT	10

#define IVI_CONTROLLER_SURFACE_VISIBILITY_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_OPACITY_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_SOURCE_RECTANGLE_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_DESTINATION_RECTANGLE_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_CONFIGURATION_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_ORIENTATION_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_PIXELFORMAT_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_LAYER_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_STATS_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_DESTROYED_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_CONTENT_SINCE_VERSION	1

static inline void
ivi_controller_surface_send_visibility(struct wl_resource *resource_, int32_t visibility)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_VISIBILITY, visibility);
}

static inline void
ivi_controller_surface_send_opacity(struct wl_resource *resource_, wl_fixed_t opacity)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_OPACITY, opacity);
}

static inline void
ivi_controller_surface_send_source_rectangle(struct wl_resource *resource_, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_SOURCE_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_surface_send_destination_rectangle(struct wl_resource *resource_, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_DESTINATION_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_surface_send_configuration(struct wl_resource *resource_, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_CONFIGURATION, width, height);
}

static inline void
ivi_controller_surface_send_orientation(struct wl_resource *resource_, int32_t orientation)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_ORIENTATION, orientation);
}

static inline void
ivi_controller_surface_send_pixelformat(struct wl_resource *resource_, int32_t pixelformat)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_PIXELFORMAT, pixelformat);
}

static inline void
ivi_controller_surface_send_layer(struct wl_resource *resource_, struct wl_resource *layer)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_LAYER, layer);
}

static inline void
ivi_controller_surface_send_stats(struct wl_resource *resource_, uint32_t redraw_count, uint32_t frame_count, uint32_t update_count, uint32_t pid, const char *process_name)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_STATS, redraw_count, frame_count, update_count, pid, process_name);
}

static inline void
ivi_controller_surface_send_destroyed(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_DESTROYED);
}

static inline void
ivi_controller_surface_send_content(struct wl_resource *resource_, int32_t content_state)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE_CONTENT, content_state);
}

/**
 * ivi_controller_layer - controller interface to layer in ivi compositor
 * @set_visibility: set visibility of layer in ivi compositor
 * @set_opacity: set opacity of layer in ivi compositor
 * @set_source_rectangle: set the scanout area of a layer in ivi
 *	compositor
 * @set_destination_rectangle: Set the destination area of a layer within
 *	a screen
 * @set_configuration: request new size for layer
 * @set_orientation: set the orientation of a layer in ivi compositor
 * @screenshot: take screenshot of layer
 * @clear_surfaces: remove all surfaces from layer render order
 * @add_surface: add a surface to layer render order at nearest
 *	z-position
 * @remove_surface: remove a surface from layer render order
 * @set_render_order: set render order of layer
 * @destroy: destroy ivi_controller_layer
 *
 * 
 */
struct ivi_controller_layer_interface {
	/**
	 * set_visibility - set visibility of layer in ivi compositor
	 * @visibility: (none)
	 *
	 * If visibility argument is 0, the layer in the ivi compositor
	 * is set to invisible. If visibility argument is not 0, the layer
	 * in the ivi compositor is set to visible.
	 */
	void (*set_visibility)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t visibility);
	/**
	 * set_opacity - set opacity of layer in ivi compositor
	 * @opacity: (none)
	 *
	 * The valid range for opacity is 0.0 (fully transparent) to 1.0
	 * (fully opaque).
	 */
	void (*set_opacity)(struct wl_client *client,
			    struct wl_resource *resource,
			    wl_fixed_t opacity);
	/**
	 * set_source_rectangle - set the scanout area of a layer in ivi
	 *	compositor
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The source rectangle defines the part of the layer content,
	 * that is used for compositing the screen. It can be used, if
	 * valid content of the layer is smaller than the layer.
	 * Effectively it can be used to zoom the content of the layer. x:
	 * horizontal start position of scanout area within the layer y:
	 * vertical start position of scanout area within the layer width:
	 * width of scanout area within the layer height: height of scanout
	 * area within the layer
	 */
	void (*set_source_rectangle)(struct wl_client *client,
				     struct wl_resource *resource,
				     int32_t x,
				     int32_t y,
				     int32_t width,
				     int32_t height);
	/**
	 * set_destination_rectangle - Set the destination area of a
	 *	layer within a screen
	 * @x: (none)
	 * @y: (none)
	 * @width: (none)
	 * @height: (none)
	 *
	 * The destination rectangle defines the position and size of a
	 * layer on a screen. The layer will be scaled to this rectangle
	 * for rendering. x: horizontal start position of layer within the
	 * screen y: vertical start position of layer within the screen
	 * width : width of surface within the screen height: height of
	 * surface within the screen
	 */
	void (*set_destination_rectangle)(struct wl_client *client,
					  struct wl_resource *resource,
					  int32_t x,
					  int32_t y,
					  int32_t width,
					  int32_t height);
	/**
	 * set_configuration - request new size for layer
	 * @width: (none)
	 * @height: (none)
	 *
	 * Layers are created with an initial size, but they can be
	 * resized at runtime. This request changes the widht and height of
	 * a layer.
	 */
	void (*set_configuration)(struct wl_client *client,
				  struct wl_resource *resource,
				  int32_t width,
				  int32_t height);
	/**
	 * set_orientation - set the orientation of a layer in ivi
	 *	compositor
	 * @orientation: (none)
	 *
	 * The orientation of a layer in ivi compositor can be rotated in
	 * 90 degree steps, as defined in orientation enum.
	 */
	void (*set_orientation)(struct wl_client *client,
				struct wl_resource *resource,
				int32_t orientation);
	/**
	 * screenshot - take screenshot of layer
	 * @filename: (none)
	 *
	 * Store a screenshot of the layer content in the file provided
	 * by argument filename.
	 */
	void (*screenshot)(struct wl_client *client,
			   struct wl_resource *resource,
			   const char *filename);
	/**
	 * clear_surfaces - remove all surfaces from layer render order
	 *
	 * A layer has no content assigned to itself, it is a container
	 * for surfaces. This request removes all surfaces from the layer
	 * render order. Note: the surfaces are not destroyed, they are
	 * just no longer contained by the layer.
	 */
	void (*clear_surfaces)(struct wl_client *client,
			       struct wl_resource *resource);
	/**
	 * add_surface - add a surface to layer render order at nearest
	 *	z-position
	 * @surface: (none)
	 *
	 * A layer has no content assigned to itself, it is a container
	 * for surfaces. This request adds a surface to the topmost
	 * position of the layer render order. The added surface will cover
	 * all other surfaces of the layer.
	 */
	void (*add_surface)(struct wl_client *client,
			    struct wl_resource *resource,
			    struct wl_resource *surface);
	/**
	 * remove_surface - remove a surface from layer render order
	 * @surface: (none)
	 *
	 * A layer has no content assigned to itself, it is a container
	 * for surfaces. This request removes one surfaces from the layer
	 * render order. Note: the surface is not destroyed, it is just no
	 * longer contained by the layer.
	 */
	void (*remove_surface)(struct wl_client *client,
			       struct wl_resource *resource,
			       struct wl_resource *surface);
	/**
	 * set_render_order - set render order of layer
	 * @id_surfaces: (none)
	 *
	 * A layer has no content assigned to itself, it is a container
	 * for surfaces. This request removes all surfaces from the layer
	 * render order and set a completely new render order.
	 */
	void (*set_render_order)(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_array *id_surfaces);
	/**
	 * destroy - destroy ivi_controller_layer
	 * @destroy_scene_object: (none)
	 *
	 * Request to destroy the ivi_controller_layer. If argument
	 * destroy_scene_object id not 0, the layer will be destroyed in
	 * ivi compositor. If argument is 0, only the proxy object is
	 * destroyed.
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource,
			int32_t destroy_scene_object);
};

#define IVI_CONTROLLER_LAYER_VISIBILITY	0
#define IVI_CONTROLLER_LAYER_OPACITY	1
#define IVI_CONTROLLER_LAYER_SOURCE_RECTANGLE	2
#define IVI_CONTROLLER_LAYER_DESTINATION_RECTANGLE	3
#define IVI_CONTROLLER_LAYER_CONFIGURATION	4
#define IVI_CONTROLLER_LAYER_ORIENTATION	5
#define IVI_CONTROLLER_LAYER_SCREEN	6
#define IVI_CONTROLLER_LAYER_DESTROYED	7

#define IVI_CONTROLLER_LAYER_VISIBILITY_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_OPACITY_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_SOURCE_RECTANGLE_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_DESTINATION_RECTANGLE_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_CONFIGURATION_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_ORIENTATION_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_SCREEN_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_DESTROYED_SINCE_VERSION	1

static inline void
ivi_controller_layer_send_visibility(struct wl_resource *resource_, int32_t visibility)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_VISIBILITY, visibility);
}

static inline void
ivi_controller_layer_send_opacity(struct wl_resource *resource_, wl_fixed_t opacity)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_OPACITY, opacity);
}

static inline void
ivi_controller_layer_send_source_rectangle(struct wl_resource *resource_, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_SOURCE_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_layer_send_destination_rectangle(struct wl_resource *resource_, int32_t x, int32_t y, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_DESTINATION_RECTANGLE, x, y, width, height);
}

static inline void
ivi_controller_layer_send_configuration(struct wl_resource *resource_, int32_t width, int32_t height)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_CONFIGURATION, width, height);
}

static inline void
ivi_controller_layer_send_orientation(struct wl_resource *resource_, int32_t orientation)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_ORIENTATION, orientation);
}

static inline void
ivi_controller_layer_send_screen(struct wl_resource *resource_, struct wl_resource *screen)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_SCREEN, screen);
}

static inline void
ivi_controller_layer_send_destroyed(struct wl_resource *resource_)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER_DESTROYED);
}

/**
 * ivi_controller_screen - controller interface to screen in ivi
 *	compositor
 * @destroy: destroy ivi_controller_screen
 * @clear: remove all layers from screen render order
 * @add_layer: add a layer to screen render order at nearest z-position
 * @screenshot: take screenshot of screen
 * @set_render_order: set render order of screen
 *
 * 
 */
struct ivi_controller_screen_interface {
	/**
	 * destroy - destroy ivi_controller_screen
	 *
	 * 
	 */
	void (*destroy)(struct wl_client *client,
			struct wl_resource *resource);
	/**
	 * clear - remove all layers from screen render order
	 *
	 * A screen has no content assigned to itself, it is a container
	 * for layers. This request removes all layers from the screen
	 * render order. Note: the layers are not destroyed, they are just
	 * no longer contained by the screen.
	 */
	void (*clear)(struct wl_client *client,
		      struct wl_resource *resource);
	/**
	 * add_layer - add a layer to screen render order at nearest
	 *	z-position
	 * @layer: (none)
	 *
	 * A screen has no content assigned to itself, it is a container
	 * for layers. This request adds a layers to the topmost position
	 * of the screen render order. The added layer will cover all other
	 * layers of the screen.
	 */
	void (*add_layer)(struct wl_client *client,
			  struct wl_resource *resource,
			  struct wl_resource *layer);
	/**
	 * screenshot - take screenshot of screen
	 * @filename: (none)
	 *
	 * Store a screenshot of the screen content in the file provided
	 * by argument filename.
	 */
	void (*screenshot)(struct wl_client *client,
			   struct wl_resource *resource,
			   const char *filename);
	/**
	 * set_render_order - set render order of screen
	 * @id_layers: (none)
	 *
	 * A screen has no content assigned to itself, it is a container
	 * for layers. This request removes all layers from the screen
	 * render order and set a completely new render order.
	 */
	void (*set_render_order)(struct wl_client *client,
				 struct wl_resource *resource,
				 struct wl_array *id_layers);
};


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
 * @commit_changes: commit all changes requested by client
 * @layer_create: create layer in ivi compositor
 * @surface_create: create surface in ivi compositor
 *
 * 
 */
struct ivi_controller_interface {
	/**
	 * commit_changes - commit all changes requested by client
	 *
	 * All requests are not applied directly to scene object, so a
	 * controller can set different properties and apply the changes
	 * all at once. Note: there's an exception to this. Creation and
	 * destruction of scene objects is executed immediately.
	 */
	void (*commit_changes)(struct wl_client *client,
			       struct wl_resource *resource);
	/**
	 * layer_create - create layer in ivi compositor
	 * @id_layer: (none)
	 * @width: (none)
	 * @height: (none)
	 * @id: (none)
	 *
	 * layer_create will create a new layer with id_layer in ivi
	 * compositor, if it does not yet exists. If the layer with
	 * id_layer already exists in ivi compositor, a handle to the
	 * existing layer is returned and width and height properties are
	 * ignored.
	 */
	void (*layer_create)(struct wl_client *client,
			     struct wl_resource *resource,
			     uint32_t id_layer,
			     int32_t width,
			     int32_t height,
			     uint32_t id);
	/**
	 * surface_create - create surface in ivi compositor
	 * @id_surface: (none)
	 * @id: (none)
	 *
	 * surface_create will create a new surface with id_surface in
	 * ivi compositor, if it does not yet exists. If the surface with
	 * id_surface already exists in ivi compositor, a handle to the
	 * existing surface is returned.
	 */
	void (*surface_create)(struct wl_client *client,
			       struct wl_resource *resource,
			       uint32_t id_surface,
			       uint32_t id);
};

#define IVI_CONTROLLER_SCREEN	0
#define IVI_CONTROLLER_LAYER	1
#define IVI_CONTROLLER_SURFACE	2
#define IVI_CONTROLLER_ERROR	3

#define IVI_CONTROLLER_SCREEN_SINCE_VERSION	1
#define IVI_CONTROLLER_LAYER_SINCE_VERSION	1
#define IVI_CONTROLLER_SURFACE_SINCE_VERSION	1
#define IVI_CONTROLLER_ERROR_SINCE_VERSION	1

static inline void
ivi_controller_send_screen(struct wl_resource *resource_, uint32_t id_screen, struct wl_resource *screen)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SCREEN, id_screen, screen);
}

static inline void
ivi_controller_send_layer(struct wl_resource *resource_, uint32_t id_layer)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_LAYER, id_layer);
}

static inline void
ivi_controller_send_surface(struct wl_resource *resource_, uint32_t id_surface)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_SURFACE, id_surface);
}

static inline void
ivi_controller_send_error(struct wl_resource *resource_, int32_t object_id, int32_t object_type, int32_t error_code, const char *error_text)
{
	wl_resource_post_event(resource_, IVI_CONTROLLER_ERROR, object_id, object_type, error_code, error_text);
}

#ifdef  __cplusplus
}
#endif

#endif
