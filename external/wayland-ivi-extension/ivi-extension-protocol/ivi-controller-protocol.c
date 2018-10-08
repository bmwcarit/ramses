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

#include <stdlib.h>
#include <stdint.h>
#include "wayland-util.h"

extern const struct wl_interface ivi_controller_layer_interface;
extern const struct wl_interface ivi_controller_screen_interface;
extern const struct wl_interface ivi_controller_surface_interface;
extern const struct wl_interface wl_output_interface;

static const struct wl_interface *types[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&ivi_controller_layer_interface,
	&ivi_controller_surface_interface,
	&ivi_controller_surface_interface,
	&wl_output_interface,
	&ivi_controller_layer_interface,
	NULL,
	NULL,
	NULL,
	&ivi_controller_layer_interface,
	NULL,
	&ivi_controller_surface_interface,
	NULL,
	&ivi_controller_screen_interface,
};

static const struct wl_message ivi_controller_surface_requests[] = {
	{ "set_visibility", "u", types + 0 },
	{ "set_opacity", "f", types + 0 },
	{ "set_source_rectangle", "iiii", types + 0 },
	{ "set_destination_rectangle", "iiii", types + 0 },
	{ "set_configuration", "ii", types + 0 },
	{ "set_orientation", "i", types + 0 },
	{ "screenshot", "s", types + 0 },
	{ "send_stats", "", types + 0 },
	{ "destroy", "i", types + 0 },
};

static const struct wl_message ivi_controller_surface_events[] = {
	{ "visibility", "i", types + 0 },
	{ "opacity", "f", types + 0 },
	{ "source_rectangle", "iiii", types + 0 },
	{ "destination_rectangle", "iiii", types + 0 },
	{ "configuration", "ii", types + 0 },
	{ "orientation", "i", types + 0 },
	{ "pixelformat", "i", types + 0 },
	{ "layer", "?o", types + 5 },
	{ "stats", "uuuu?s", types + 0 },
	{ "destroyed", "", types + 0 },
	{ "content", "i", types + 0 },
};

WL_EXPORT const struct wl_interface ivi_controller_surface_interface = {
	"ivi_controller_surface", 1,
	9, ivi_controller_surface_requests,
	11, ivi_controller_surface_events,
};

static const struct wl_message ivi_controller_layer_requests[] = {
	{ "set_visibility", "u", types + 0 },
	{ "set_opacity", "f", types + 0 },
	{ "set_source_rectangle", "iiii", types + 0 },
	{ "set_destination_rectangle", "iiii", types + 0 },
	{ "set_configuration", "ii", types + 0 },
	{ "set_orientation", "i", types + 0 },
	{ "screenshot", "s", types + 0 },
	{ "clear_surfaces", "", types + 0 },
	{ "add_surface", "o", types + 6 },
	{ "remove_surface", "o", types + 7 },
	{ "set_render_order", "a", types + 0 },
	{ "destroy", "i", types + 0 },
};

static const struct wl_message ivi_controller_layer_events[] = {
	{ "visibility", "i", types + 0 },
	{ "opacity", "f", types + 0 },
	{ "source_rectangle", "iiii", types + 0 },
	{ "destination_rectangle", "iiii", types + 0 },
	{ "configuration", "ii", types + 0 },
	{ "orientation", "i", types + 0 },
	{ "screen", "?o", types + 8 },
	{ "destroyed", "", types + 0 },
};

WL_EXPORT const struct wl_interface ivi_controller_layer_interface = {
	"ivi_controller_layer", 1,
	12, ivi_controller_layer_requests,
	8, ivi_controller_layer_events,
};

static const struct wl_message ivi_controller_screen_requests[] = {
	{ "destroy", "", types + 0 },
	{ "clear", "", types + 0 },
	{ "add_layer", "o", types + 9 },
	{ "screenshot", "s", types + 0 },
	{ "set_render_order", "a", types + 0 },
};

WL_EXPORT const struct wl_interface ivi_controller_screen_interface = {
	"ivi_controller_screen", 1,
	5, ivi_controller_screen_requests,
	0, NULL,
};

static const struct wl_message ivi_controller_requests[] = {
	{ "commit_changes", "", types + 0 },
	{ "layer_create", "uiin", types + 10 },
	{ "surface_create", "un", types + 14 },
};

static const struct wl_message ivi_controller_events[] = {
	{ "screen", "un", types + 16 },
	{ "layer", "u", types + 0 },
	{ "surface", "u", types + 0 },
	{ "error", "iii?s", types + 0 },
};

WL_EXPORT const struct wl_interface ivi_controller_interface = {
	"ivi_controller", 1,
	3, ivi_controller_requests,
	4, ivi_controller_events,
};

