/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to GLES2.
 * Kristian Høgsberg <krh@bitplanet.net>
 * May 3, 2010
 */

/*
 * MFiedler:
 * - fetched from mesa-demos-8.0.1
 * - removed dependencies to eglut
 * - introduced extern functions for EGL display and native window.
 */

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES


#define _DEFAULT_SOURCE

#include <wayland-client.h>
#include <wayland-egl.h>
#include <ivi-application-client-protocol.h>
#include <ivi-controller-client-protocol.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>

#define UNUSED(x) {(void)(x);}

extern EGLDisplay GetDisplay(int argc, char *argv[]);
extern NativeWindowType GetNativeWindow(int argc, char *argv[]);

EGLDisplay egldisplay;
EGLConfig eglconfig;
EGLSurface eglsurface;
EGLContext eglcontext;

typedef struct {
   struct wl_display *display;
   struct wl_registry *registry;
   struct wl_compositor *compositor;
   struct wl_shell *shell;
   struct wl_seat *seat;
   struct wl_pointer *pointer;
   struct wl_surface *focused_surface;
   struct ivi_application *ivi_app;
   struct ivi_controller *ivi_controller;
   int fd;
} wayland_display_t;

typedef struct {
   int width;
   int height;
   struct wl_egl_window *native;
   struct wl_surface *surface;
   struct ivi_surface *ivi_surf;
} wayland_window_t;

struct gear {
   GLfloat *vertices;
   GLuint vbo;
   int count;
};

static wayland_display_t wayland;
static wayland_window_t window;

static uint32_t ivi_surface_id = 0;
static uint32_t ivi_layer_id = 0;

static int gettime(void)
{
   struct timeval tv;
   (void)gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#define EGL_ERROR_CASE(E, F) case E: fprintf(stderr, "error %s at %s\n", #E, F); break

static int checkEGLError(const char *function)
{
   int err = eglGetError();
   switch (err) {
   case EGL_SUCCESS:
      break;
      EGL_ERROR_CASE(EGL_NOT_INITIALIZED, function);
      EGL_ERROR_CASE(EGL_BAD_ACCESS, function);
      EGL_ERROR_CASE(EGL_BAD_ALLOC, function);
      EGL_ERROR_CASE(EGL_BAD_ATTRIBUTE, function);
      EGL_ERROR_CASE(EGL_BAD_CONTEXT, function);
      EGL_ERROR_CASE(EGL_BAD_CONFIG, function);
      EGL_ERROR_CASE(EGL_BAD_CURRENT_SURFACE, function);
      EGL_ERROR_CASE(EGL_BAD_DISPLAY, function);
      EGL_ERROR_CASE(EGL_BAD_SURFACE, function);
      EGL_ERROR_CASE(EGL_BAD_MATCH, function);
      EGL_ERROR_CASE(EGL_BAD_PARAMETER, function);
      EGL_ERROR_CASE(EGL_BAD_NATIVE_PIXMAP, function);
      EGL_ERROR_CASE(EGL_BAD_NATIVE_WINDOW, function);
      EGL_ERROR_CASE(EGL_CONTEXT_LOST, function);
   default:
      fprintf(stderr, "unknown error 0x%X at %s\n", (unsigned)(err), function);
   }
   return err == EGL_SUCCESS ? 0 : err;
}

#define CHECK_EGL(E) \
   if (!(E)) { checkEGLError(#E); return -1; }

static int setupEGL(int verbose)
{
   static const EGLint s_configAttribs[] = {
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,   // add this, otherwise EGL might not get any configs
      EGL_RED_SIZE, 1,
      EGL_GREEN_SIZE, 1,
      EGL_BLUE_SIZE, 1,
      EGL_ALPHA_SIZE, 1,
      EGL_DEPTH_SIZE, 24, // 16 bit does not work on the Intel NUC VDT for unknown reason
      EGL_NONE
   };
   EGLint ContextAttribList[] =
       { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   EGLint numconfigs;

   egldisplay = eglGetDisplay((EGLNativeDisplayType) wayland.display);
   if (verbose) printf("Display: %p\n", egldisplay);
   int major, minor;
   CHECK_EGL(eglInitialize(egldisplay, &major, &minor) == EGL_TRUE);
   if (verbose) printf("EGL: %i.%i\n", major, minor);
   CHECK_EGL(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
   CHECK_EGL(eglChooseConfig
        (egldisplay, s_configAttribs, &eglconfig, 1,
         &numconfigs) == EGL_TRUE);

   //assert(numconfigs == 1);
   if (numconfigs == 0) {
      fprintf(stderr, "got no EGL configs\n");
      return 1;
   }

   if (verbose) printf("EGLConfig: %p\n", eglconfig);

   CHECK_EGL((eglcontext =
         eglCreateContext(egldisplay, eglconfig, EGL_NO_CONTEXT,
                ContextAttribList)) != EGL_NO_CONTEXT);

   return 0;
}

static
void window_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
   UNUSED(wl_pointer);
   UNUSED(serial);
   UNUSED(surface);
   UNUSED(surface_x);
   UNUSED(surface_y);
   wayland_display_t* wl = (wayland_display_t*)data;
   wl->focused_surface = surface;
}

static
void window_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
   UNUSED(wl_pointer);
   UNUSED(serial);
   UNUSED(surface);
   wayland_display_t* wl = (wayland_display_t*)data;
   wl->focused_surface = NULL;
}

static
void window_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
   UNUSED(data);
   UNUSED(wl_pointer);
   UNUSED(time);
   UNUSED(surface_x);
   UNUSED(surface_y);
}

static
void window_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
   UNUSED(pointer);
   UNUSED(time);
   UNUSED(button);
   wayland_display_t* wl = (wayland_display_t*)data;
   struct wl_shell_surface *shell_surface = wl_surface_get_user_data(wl->focused_surface);
   if (state)
   {
      wl_shell_surface_move(shell_surface, wl->seat, serial);
   }
}

static
void window_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
   UNUSED(data);
   UNUSED(wl_pointer);
   UNUSED(time);
   UNUSED(axis);
   UNUSED(value);
}


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static
const struct wl_pointer_listener pointer_listener =
{
      window_handle_enter,
      window_handle_leave,
      window_handle_motion,
      window_handle_button,
      window_handle_axis
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void
seat_handle_capabilities(void *data, struct wl_seat *seat,
          enum wl_seat_capability caps)
{
   wayland_display_t *d = data;

   if ((caps & WL_SEAT_CAPABILITY_POINTER) && !d->pointer) {
      d->pointer = wl_seat_get_pointer(seat);
      wl_pointer_add_listener(d->pointer, &pointer_listener, d);
   } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && d->pointer) {
      wl_pointer_destroy(d->pointer);
      d->pointer = NULL;
   }
}

static void ivi_controller_handle_screen(void*                         data,
                                              struct ivi_controller*        controller,
                                              uint32_t                      id_screen,
                                              struct ivi_controller_screen* screen)
{
    UNUSED(data)
    UNUSED(controller)
    UNUSED(screen)
    printf("ivi_controller_handle_layer id_layer: %u\n", id_screen);
}

static void ivi_controller_handle_layer(void* data, struct ivi_controller* controller, uint32_t id_layer)
{
    UNUSED(data)
    UNUSED(controller)
    printf("ivi_controller_handle_layer id_layer: %u\n", id_layer);
}

static void ivi_controller_handle_surface(void* data, struct ivi_controller* controller, uint32_t id_surface)
{
    UNUSED(data)
    UNUSED(controller)
    printf("ivi_controller_handle_surface id_surface: %u\n", id_surface);
}

static void ivi_controller_handler_error(void*                  data,
                                         struct ivi_controller* controller,
                                         int32_t                object_id,
                                         int32_t                object_type,
                                         int32_t                error_code,
                                         const char*            error_text)
{
    UNUSED(data)
    UNUSED(controller)
    UNUSED(object_id)
    UNUSED(object_type)
    UNUSED(error_code)
    UNUSED(error_text)

    printf("ivi_controller_handler_error error_text: %s\n", error_text);
}

static void make_surface_visible()
{
    if (ivi_layer_id > 0 && wayland.ivi_controller)
    {
        struct ivi_controller_layer* controllerLayer =
            ivi_controller_layer_create(wayland.ivi_controller, ivi_layer_id, 0, 0);
        struct ivi_controller_surface* controllerSurface =
            ivi_controller_surface_create(wayland.ivi_controller, ivi_surface_id);

        ivi_controller_layer_add_surface(controllerLayer, controllerSurface);
        ivi_controller_surface_set_visibility(controllerSurface, 1);

        ivi_controller_commit_changes(wayland.ivi_controller);
        ivi_controller_layer_destroy(controllerLayer, 0);
        ivi_controller_surface_destroy(controllerSurface, 0);
        wl_display_flush(wayland.display);
    }
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static const struct wl_seat_listener seat_listener = {
   seat_handle_capabilities
};

static const struct ivi_controller_listener controller_listener = {
    ivi_controller_handle_screen,
    ivi_controller_handle_layer,
    ivi_controller_handle_surface,
    ivi_controller_handler_error
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void
registry_handle_global(void *data, struct wl_registry *registry,
             uint32_t name, const char *interface, uint32_t version)
{
   UNUSED(version);
   wayland_display_t *d = data;

   if (strcmp(interface, "wl_compositor") == 0) {
      d->compositor =
         wl_registry_bind(registry, name,
                          &wl_compositor_interface, 1);
   } else if (strcmp(interface, "wl_shell") == 0) {
      d->shell = wl_registry_bind(registry, name,
                   &wl_shell_interface, 1);
   } else if (strcmp(interface, "wl_seat") == 0) {
      d->seat = wl_registry_bind(registry, name,
                  &wl_seat_interface, 1);
      wl_seat_add_listener(d->seat, &seat_listener, d);
   } else if (strcmp(interface, "ivi_application") == 0) {
      d->ivi_app = wl_registry_bind(registry, name,
                  &ivi_application_interface, 1);
  } else if (strcmp(interface, "ivi_controller") == 0) {
      d->ivi_controller = wl_registry_bind(registry, name, &ivi_controller_interface, 1);
      ivi_controller_add_listener(d->ivi_controller, &controller_listener, 0);
  }
}

static void
registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
    UNUSED(data)
    UNUSED(registry)
    UNUSED(name)
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static const struct wl_registry_listener registry_listener = {
   registry_handle_global,
   registry_handle_global_remove
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static void create_ivi_surface(void)
{
    if (ivi_surface_id > 0)
    {
        printf("create_ivi_surface with id: %u\n", ivi_surface_id);
        if (wayland.ivi_app)
        {
            printf("wayland.ivi_app is valid, registering\n");
            window.ivi_surf = ivi_application_surface_create(wayland.ivi_app,
                                                             ivi_surface_id, window.surface);

            if (window.ivi_surf == NULL)
            {
                fprintf(stderr, "Failed to create ivi_client_surface\n");
                abort();
            }
        }
    }
}

static int setupWayland(void)
{
   wayland.display = 0;
   wayland.registry = 0;
   wayland.compositor = 0;
   wayland.shell = 0;
   wayland.seat = 0;
   wayland.pointer = 0;
   wayland.focused_surface = 0;
   wayland.ivi_app = 0;
   wayland.fd = 0;

   wayland.display = wl_display_connect(NULL);
   if (wayland.display == 0) {
      fprintf(stderr, "wl_display_connect() failed\n");
      return -1;
   }

   wayland.registry = wl_display_get_registry(wayland.display);
   wl_registry_add_listener(wayland.registry,
             &registry_listener, &wayland);

   wayland.fd = wl_display_get_fd(wayland.display);

   wl_display_dispatch(wayland.display);
   wl_display_roundtrip(wayland.display);

   return 0;
}

static void closeWayland(void)
{
   if (wayland.ivi_app)
   {
      ivi_application_destroy(wayland.ivi_app);
   }
   if (wayland.pointer)
   {
       wl_pointer_destroy(wayland.pointer);
   }
   if (wayland.seat)
   {
       wl_seat_destroy(wayland.seat);
   }
   if (wayland.shell)
   {
       wl_shell_destroy(wayland.shell);
   }
   if (wayland.compositor)
   {
       wl_compositor_destroy(wayland.compositor);
   }
   if (wayland.registry)
   {
       wl_registry_destroy(wayland.registry);
   }
   wl_display_roundtrip(wayland.display);
   wl_display_disconnect(wayland.display);
}

static int createWindow(void)
{
   window.native = 0;
   window.surface = 0;
   window.ivi_surf = 0;

   struct wl_shell_surface *shsurf = 0;

   window.surface = wl_compositor_create_surface(wayland.compositor);

   if (wayland.shell)
   {
       shsurf = wl_shell_get_shell_surface(wayland.shell, window.surface);
       wl_surface_set_user_data(window.surface, shsurf);
   }

   window.native = wl_egl_window_create(window.surface, window.width, window.height);

   create_ivi_surface();

   eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, (NativeWindowType)window.native, NULL);
   CHECK_EGL(eglsurface != EGL_NO_SURFACE);

   if (shsurf)
   {
       wl_shell_surface_set_title(shsurf, "gears");
       wl_shell_surface_set_toplevel(shsurf);
   }

   CHECK_EGL((eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext)) == EGL_TRUE);

   return 0;
}

static void terminateEGL(void)
{
   eglTerminate(egldisplay);
}

static void destroyWindow()
{
   eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
             EGL_NO_CONTEXT);
   eglDestroyContext(egldisplay, eglcontext);
   eglDestroySurface(egldisplay, eglsurface);

   if (window.ivi_surf != 0)
   {
       ivi_surface_destroy(window.ivi_surf);
   }
   wl_egl_window_destroy(window.native);
   wl_surface_destroy(window.surface);
}

static
int check_events(void)
{
   struct pollfd pfd;

   pfd.fd = wayland.fd;
   pfd.events = POLLIN;
   pfd.revents = 0;

   wl_display_dispatch_pending(wayland.display);
   wl_display_flush(wayland.display);

   if (poll(&pfd, 1, 0) == -1)
      return -1;

   if (pfd.revents & POLLIN) {
      return wl_display_dispatch(wayland.display);
   }

   return 0;
}

/*----------------------------------------------------------------------------------------------*/

typedef struct {
   float e[3];
} vect3x32;

struct vertex {
   vect3x32 pos;
   vect3x32 norm;
};

struct trilist {
   struct vertex *vertices;
   unsigned short vcount;

   unsigned short *indices;
   unsigned short icount;
};

#define DEG2RAD 0.017453292519943295769236907684886f
static const float deg2rad = DEG2RAD;

static const float halfpi = 1.5707963267948966192313216916398f;
// static const float pi = 3.1415926535897932384626433832795f;
static const float twopi = 6.283185307179586476925286766559f;

static GLfloat view_rotx = 20.0f * DEG2RAD, view_roty =
    30.0f * DEG2RAD, view_rotz = 0.0f;
static GLfloat angle = 0.0f;

/**

  Changes for ES 2.0 Compliance:

    * Store one OpenGL program number.

    * Use two buffers to store vertex data and index data.

    * Three trilist structures will be used to gather geometry so that it can be
      copied to a vertex buffer object.

    * Store the locations of all the program uniforms.

    * The fixed function transformation pipeline is not supported as per ES 2.0 spec para 2.11.
      Matrices are required to perform the same transformations without the OpenGL matrix stack.

 **/

/* there will be only one program for this demo */
static GLuint program;

/* two buffers: one for the vertex data and one for the indices */
static GLuint buffers[2];

/* three triangle lists for each spinning gear */
static struct trilist gear1, gear2, gear3;

/* location of the ModelViewProjectionMatrix uniform used in the vertex shader */
static GLuint mvp_matrix_loc;

/* location of the NormalMatrix uniform used in the vertex shader */
static GLuint normal_matrix_loc;

/* location of the LightPosition uniform used in the vertex shader */
static GLuint light_pos_loc;

/* location of the LightModelProduct uniform used in the vertex shader */
static GLuint lm_prod_loc;

/* location of the AmbientDiffuse uniform used in the vertex shader */
static GLuint ambient_diffuse_loc;

/* used for assignments in translation and rotation functions */
static const GLfloat Identity[16] = {
   1.0f, 0.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f, 0.0f,
   0.0f, 0.0f, 1.0f, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f
};

/* updated for each gear */
static GLfloat ModelViewProjectionMatrix[16];

/* updated when the window is reshaped */
static GLfloat ProjectionMatrix[16];

/* updated everytime the view matrix changes (keystrokes) */
static GLfloat ViewMatrix[16];

/* OpenGL can't extract a 3x3 matrix out of a 4x4 matrix with Uniform[] calls.
   We need a separate matrix that will store the upper left 3x3 portion of the ModelView matrix. */
static GLfloat NormalMatrix[9];

/**

  Simply copies the vertex position and normal coefficients to the array
  element pointed to by the count pointer. The function also increments the
  value pointed to by count so that add_vertex can be called sequentially
  to add vertices.

 **/

static void add_vertex(struct vertex *vertices, unsigned short *count,
             const struct vertex *v)
{
   vertices[*count].pos.e[0] = v->pos.e[0];
   vertices[*count].pos.e[1] = v->pos.e[1];
   vertices[*count].pos.e[2] = v->pos.e[2];
   vertices[*count].norm.e[0] = v->norm.e[0];
   vertices[*count].norm.e[1] = v->norm.e[1];
   vertices[*count].norm.e[2] = v->norm.e[2];
   ++(*count);
}

/**

  Draw a gear wheel.  You'll probably want to call this function when
  building a display list since we do a lot of trig here.

  Input:  inner_radius - radius of hole at center
          outer_radius - radius at center of teeth
          width - width of gear
          teeth - number of teeth
          tooth_depth - depth of tooth

 **/

/**

  Changes for ES 2.0 compliance:

    * ShadeModel is not supported as per ES 2.0 spec para 2.14.
      Flat shading can be implemented by duplicating shared vertices so that they each
      have a normal that corresponds to the face normal.

    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      Immediate mode is not supported as per ES 2.0 spec para 2.6.
      We cannot use the notion of current vertex as per ES 2.0 spec para 2.7.
      Instead, the vertex positions and normals--one for each position--will be accumulated
      in a buffer to be used as a vertex array. The vertex buffer data will also
      be stored in a vertex buffer object for better performance.

    * Indexed drawing is used to improve performance by reducing the number of
      vertices transformed from 2160 to 1040, which is less than half.
      Indexed drawing is supported as per ES 2.0 spec para 2.8.

    * GL_QUADS and GL_QUAD_STRIP are not supported as per ES 2.0 spec para 2.6.
      The primitives were replaced by triangles.

    * The indices were manually gathered to avoid expensive searches for matching
      vertices.

 **/

static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
       GLint teeth, GLfloat tooth_depth, GLushort index_offset,
       struct trilist *triangles)
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat ang, da, inc;
   GLfloat u1, v1, u2, v2, invlen;
   GLfloat cos_angle, sin_angle;
   GLfloat cos_angle_1da, sin_angle_1da;
   GLfloat cos_angle_2da, sin_angle_2da;
   GLfloat cos_angle_3da, sin_angle_3da;
   GLfloat half_width;
   struct vertex v;
   unsigned short start;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth * 0.5f;
   r2 = outer_radius + tooth_depth * 0.5f;

   da = halfpi / teeth;
   inc = twopi / teeth;
   half_width = width * 0.5f;

   /* we know in advance that there will be 30 vertices per tooth + 4 */
   triangles->vertices = malloc((28 * teeth + 4) * sizeof(struct vertex));
   triangles->vcount = 0;

   /* we know in advance that there are 20 triangles per tooth + 2 */
   triangles->indices = malloc((60 + 6) * teeth * sizeof(unsigned short));
   triangles->icount = 0;

   /* all flat shading loops were collapsed into one */
   for (i = 0, ang= 0.0f; i < teeth; i++, ang += inc) {
      /* pre-compute these because they will be reused a lot */
      cos_angle = cos(ang);
      sin_angle = sin(ang);
      cos_angle_1da = cos(ang + da);
      sin_angle_1da = sin(ang + da);
      cos_angle_2da = cos(ang + 2.0f * da);
      sin_angle_2da = sin(ang + 2.0f * da);
      cos_angle_3da = cos(ang + 3.0f * da);
      sin_angle_3da = sin(ang + 3.0f * da);

      /* u1 and v1 are not texture coordinates but normal components */
      u1 = r2 * cos_angle_1da - r1 * cos_angle;
      v1 = r2 * sin_angle_1da - r1 * sin_angle;
      invlen = 1.0f / sqrt(u1 * u1 + v1 * v1);
      u1 *= invlen;
      v1 *= invlen;

      /* u2 and v2 are not texture coordinates but normal components */
      u2 = r1 * cos_angle_3da - r2 * cos_angle_2da;
      v2 = r1 * sin_angle_3da - r2 * sin_angle_2da;
      invlen = 1.0f / sqrt(u2 * u2 + v2 * v2);
      u2 *= invlen;
      v2 *= invlen;

   /*******************************************************/
      /* triangles from inner radius to outer radius (right) */

      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 1;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 2;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 2;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 0 : triangles->vcount + 26);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 2;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 1 : triangles->vcount + 27);
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 0 : triangles->vcount + 26);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 1;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 3;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 4;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 1;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 4;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 2;

   /******************************************************/
      /* triangles from inner radius to outer radius (left) */

      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 5;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 7;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 6;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 5;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 5 : triangles->vcount + 31);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 7;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 7;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 5 : triangles->vcount + 31);
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 6 : triangles->vcount + 32);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 6;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 9;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 8;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 6;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 7;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 9;

   /******************************/
      /* outside faces of the wheel */

      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 10;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 11;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 12;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 12;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 11;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 13;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 14;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 15;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 16;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 16;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 15;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 17;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 18;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 19;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 20;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 20;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 19;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 21;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 22;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 23;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 24 : triangles->vcount + 50);
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 24 : triangles->vcount + 50);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 23;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth - 1 ? 25 : triangles->vcount + 51);

   /************************************/
      /* a list of vertices for this loop */

      /* vcount + 0 */
      v.pos.e[0] = r0 * cos_angle;
      v.pos.e[1] = r0 * sin_angle;
      v.pos.e[2] = half_width;
      v.norm.e[0] = 0.0f;
      v.norm.e[1] = 0.0f;
      v.norm.e[2] = 1.0f;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 1 */
      v.pos.e[0] = r1 * cos_angle;
      v.pos.e[1] = r1 * sin_angle;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 2 */
      v.pos.e[0] = r1 * cos_angle_3da;
      v.pos.e[1] = r1 * sin_angle_3da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 3 */
      v.pos.e[0] = r2 * cos_angle_1da;
      v.pos.e[1] = r2 * sin_angle_1da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 4 */
      v.pos.e[0] = r2 * cos_angle_2da;
      v.pos.e[1] = r2 * sin_angle_2da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 5 */
      v.pos.e[0] = r0 * cos_angle;
      v.pos.e[1] = r0 * sin_angle;
      v.pos.e[2] = -half_width;
      v.norm.e[2] = -1.0f;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 6 */
      v.pos.e[0] = r1 * cos_angle;
      v.pos.e[1] = r1 * sin_angle;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 7 */
      v.pos.e[0] = r1 * cos_angle_3da;
      v.pos.e[1] = r1 * sin_angle_3da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 8 */
      v.pos.e[0] = r2 * cos_angle_1da;
      v.pos.e[1] = r2 * sin_angle_1da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 9 */
      v.pos.e[0] = r2 * cos_angle_2da;
      v.pos.e[1] = r2 * sin_angle_2da;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 10 */
      v.pos.e[0] = r1 * cos_angle;
      v.pos.e[1] = r1 * sin_angle;
      v.pos.e[2] = half_width;
      v.norm.e[0] = v1;
      v.norm.e[1] = -u1;
      v.norm.e[2] = 0.0f;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 11 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 12 */
      v.pos.e[0] = r2 * cos_angle_1da;
      v.pos.e[1] = r2 * sin_angle_1da;
      v.pos.e[2] = half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 13 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 14 */
      v.pos.e[0] = r2 * cos_angle_1da;
      v.pos.e[1] = r2 * sin_angle_1da;
      v.pos.e[2] = half_width;
      v.norm.e[0] = cos_angle;
      v.norm.e[1] = sin_angle;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 15 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 16 */
      v.pos.e[0] = r2 * cos_angle_2da;
      v.pos.e[1] = r2 * sin_angle_2da;
      v.pos.e[2] = half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 17 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 18 */
      v.pos.e[0] = r2 * cos_angle_2da;
      v.pos.e[1] = r2 * sin_angle_2da;
      v.pos.e[2] = half_width;
      v.norm.e[0] = v2;
      v.norm.e[1] = -u2;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 19 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 20 */
      v.pos.e[0] = r1 * cos_angle_3da;
      v.pos.e[1] = r1 * sin_angle_3da;
      v.pos.e[2] = half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 21 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 22 */
      v.pos.e[0] = r1 * cos_angle_3da;
      v.pos.e[1] = r1 * sin_angle_3da;
      v.pos.e[2] = half_width;
      v.norm.e[0] = cos_angle;
      v.norm.e[1] = sin_angle;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 23 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 24 */
      v.pos.e[0] = r1 * cos_angle;
      v.pos.e[1] = r1 * sin_angle;
      v.pos.e[2] = half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 25 */
      v.pos.e[2] = -half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);
   }

   start = triangles->vcount;

   /* inside radius cylinder */
   for (i = 0, ang = 0.0f; i <= teeth; i++, ang += inc) {
      cos_angle = cos(ang);
      sin_angle = sin(ang);

      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount;
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 1;
      triangles->indices[triangles->icount++] =
          index_offset + (i == teeth ? start : triangles->vcount + 2);
      triangles->indices[triangles->icount++] =
          index_offset + triangles->vcount + 1;
      triangles->indices[triangles->icount++] =
          index_offset + (i ==
                teeth ? start + 1 : triangles->vcount + 3);
      triangles->indices[triangles->icount++] =
          index_offset + (i == teeth ? start : triangles->vcount + 2);

   /************************************/
      /* a list of vertices for this loop */

      /* vcount + 0 */
      v.pos.e[0] = r0 * cos_angle;
      v.pos.e[1] = r0 * sin_angle;
      v.pos.e[2] = -half_width;
      v.norm.e[0] = -cos_angle;
      v.norm.e[1] = -sin_angle;
      v.norm.e[2] = 0.0f;
      add_vertex(triangles->vertices, &triangles->vcount, &v);

      /* vcount + 1 */
      v.pos.e[2] = half_width;
      add_vertex(triangles->vertices, &triangles->vcount, &v);
   }
}

/**

  This function creates an OpenGL program that has one vertex shader and one
  fragment shader sourced from the gears.vs and gears.fs respectively. The function
  also fetches the uniform locations in some global variables so that their values
  can be changed later on.

 **/

static void load_program(void)
{
   const char *vtx_shdr_src =
       "// gears.vs\n"
       "//\n"
       "// Emulates a fixed-function pipeline with:\n"
       "//  GL_ALPHA_TEST is disabled by default\n"
       "//  GL_BLEND is disabled by default\n"
       "//  GL_CLIP_PLANEi is disabled\n"
       "//  GL_LIGHTING and GL_LIGHT0 enabled\n"
       "//  GL_FOG disabled\n"
       "//  GL_TEXTURE_xx disabled\n"
       "//  GL_TEXTURE_GEN_x disabled\n"
       "//\n"
       "//  GL_LIGHT_MODEL_AMBIENT is never set -> default value is (0.2, 0.2, 0.2, 1.0)\n"
       "//\n"
       "//  GL_LIGHT0 position is (5, 5, 10, 0)\n"
       "//  GL_LIGHT0 ambient is never set -> default value is (0, 0, 0, 1)\n"
       "//  GL_LIGHT0 diffuse is never set -> default value is (1, 1, 1, 1)\n"
       "//  GL_LIGHT0 specular is never set -> default value is (1, 1, 1, 1)\n"
       "//\n"
       "//  GL_AMBIENT and GL_DIFFUSE are (red/green/blue)\n"
       "//  GL_SPECULAR is never set -> default value is (0, 0, 0, 1)\n"
       "//  GL_EMISSION is never set -> default value is (0, 0, 0, 1)\n"
       "//  GL_SHININESS is never set -> default value is 0\n"
       "//\n"
       "//  ES 2.0 only supports generic vertex attributes as per spec para 2.6\n"
       "//\n"
       "//  Combining material with light settings gives us directional diffuse with\n"
       "//  ambient from light model.\n"
       "//\n"
       "//  Since alpha test and blend are both disabled, there is no need to keep track\n"
       "//  of the alpha component when shading. This saves the interpolation of one\n"
       "//  float between vertices and brings the performance back to the level of the\n"
       "//  'fixed function pipeline' when the window is maximized.\n"
       "//\n"
       "//  App is responsible for normalizing LightPosition.\n"
       "\n"
       "// should be set to gl_ProjectionMatrix * gl_ModelViewMatrix\n"
       "uniform mat4 ModelViewProjectionMatrix;\n"
       "\n"
       "// should be same as mat3(gl_ModelViewMatrix) because gl_ModelViewMatrix is orthogonal\n"
       "uniform mat3 NormalMatrix;\n"
       "\n"
       "// should be set to 'normalize(5.0, 5.0, 10.0)'\n"
       "uniform vec3 LightPosition;\n"
       "\n"
       "// should be set to AmbientDiffuse * vec4(0.2, 0.2, 0.2, 1.0)\n"
       "uniform vec3 LightModelProduct;\n"
       "\n"
       "// will be set to red/green/blue\n"
       "uniform vec3 AmbientDiffuse;\n"
       "\n"
       "// user defined attribute to replace gl_Vertex\n"
       "attribute vec4 Vertex;\n"
       "\n"
       "// user defined attribute to replace gl_Normal\n"
       "attribute vec3 Normal;\n"
       "\n"
       "// interpolate per-vertex lighting\n"
       "varying vec3 Color;\n"
       "\n"
       "void main(void)\n"
       "{\n"
       "  // transform the vertex and normal in eye space\n"
       "  gl_Position = ModelViewProjectionMatrix * Vertex;\n"
       "  vec3 NormEye = normalize(NormalMatrix * Normal);\n"
       "\n"
       "  // do one directional light (diffuse only); no need to clamp for this example\n"
       "  Color = LightModelProduct + AmbientDiffuse * max(0.0, dot(NormEye, LightPosition));\n"
       "}\n";

   const char *frg_shdr_src =
       "// gears.fs\n"
       "//\n"
       "// Emulates a fixed function pipeline with:\n"
       "//  GL_ALPHA_TEST is disabled by default\n"
       "//  GL_BLEND is disabled by default\n"
       "//  GL_FOG disabled\n"
       "//  GL_TEXTURE_xx disabled\n"
       "//\n"
       "//  Since alpha test and blend are both disabled, there is no need to keep track\n"
       "//  of the alpha component in the fragment shader.\n"
       "\n"
       "// interpolated per-vertex lighting\n"
       "varying lowp vec3 Color;\n"
       "\n"
       "void main(void)\n"
       "{\n"
       "  // this is the simplest fragment shader\n"
       "  gl_FragColor = vec4(Color, 1.0);\n" "}\n";

   GLuint vertex_shader, fragment_shader;

   program = glCreateProgram();

   vertex_shader = glCreateShader(GL_VERTEX_SHADER);
   glAttachShader(program, vertex_shader);
   glShaderSource(vertex_shader, 1, (const char **)&vtx_shdr_src, NULL);
   glCompileShader(vertex_shader);

   fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
   glAttachShader(program, fragment_shader);
   glShaderSource(fragment_shader, 1, (const char **)&frg_shdr_src, NULL);
   glCompileShader(fragment_shader);

   glLinkProgram(program);
   glUseProgram(program);

   mvp_matrix_loc =
       glGetUniformLocation(program, "ModelViewProjectionMatrix");
   normal_matrix_loc = glGetUniformLocation(program, "NormalMatrix");
   light_pos_loc = glGetUniformLocation(program, "LightPosition");
   lm_prod_loc = glGetUniformLocation(program, "LightModelProduct");
   ambient_diffuse_loc = glGetUniformLocation(program, "AmbientDiffuse");
}

/**

  Creates a perspective matrix as per OpenGL definition of glFrustum.

 **/

static void frustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
          GLfloat znear, GLfloat zfar)
{
   ProjectionMatrix[0] = 2.0f * znear / (right - left);
   ProjectionMatrix[1] = 0.0f;
   ProjectionMatrix[2] = 0.0f;
   ProjectionMatrix[3] = 0.0f;
   ProjectionMatrix[4] = 0.0f;
   ProjectionMatrix[5] = 2.0f * znear / (top - bottom);
   ProjectionMatrix[6] = 0.0f;
   ProjectionMatrix[7] = 0.0f;
   ProjectionMatrix[8] = (right + left) / (right - left);
   ProjectionMatrix[9] = (top + bottom) / (top - bottom);
   ProjectionMatrix[10] = -(zfar + znear) / (zfar - znear);
   ProjectionMatrix[11] = -1.0f;
   ProjectionMatrix[12] = 0.0f;
   ProjectionMatrix[13] = 0.0f;
   ProjectionMatrix[14] = -2.0f * zfar * znear / (zfar - znear);
   ProjectionMatrix[15] = 0.0f;
}

/**

  Copies the upper 3x3 matrix of 4x4 matrix B into 3x3 matrix A.
  The result is also the return value to allow cascading matrix operations.

 **/

static GLfloat *matrix_copy_3x3(GLfloat * A, const GLfloat * B)
{
   A[0] = B[0];
   A[1] = B[1];
   A[2] = B[2];
   A[3] = B[4];
   A[4] = B[5];
   A[5] = B[6];
   A[6] = B[8];
   A[7] = B[9];
   A[8] = B[10];
   return A;
}

/**

  Stores the multiplication of matrix A and B into matrix B.
  The result is also the return value to allow cascading matrix operations.

 **/

static GLfloat *matrix_rmultiply(const GLfloat * A, GLfloat * B)
{
   GLfloat T[3];

   T[0] = B[0];
   T[1] = B[1];
   T[2] = B[2];

   B[0] = A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3];
   B[1] = A[1] * T[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3];
   B[2] = A[2] * T[0] + A[6] * T[1] + A[10] * B[2] + A[14] * B[3];
   B[3] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[3];

   T[0] = B[4];
   T[1] = B[5];
   T[2] = B[6];

   B[4] = A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7];
   B[5] = A[1] * T[0] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7];
   B[6] = A[2] * T[0] + A[6] * T[1] + A[10] * B[6] + A[14] * B[7];
   B[7] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[7];

   T[0] = B[8];
   T[1] = B[9];
   T[2] = B[10];

   B[8] = A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11];
   B[9] = A[1] * T[0] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11];
   B[10] = A[2] * T[0] + A[6] * T[1] + A[10] * B[10] + A[14] * B[11];
   B[11] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[11];

   T[0] = B[12];
   T[1] = B[13];
   T[2] = B[14];

   B[12] = A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15];
   B[13] = A[1] * T[0] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15];
   B[14] = A[2] * T[0] + A[6] * T[1] + A[10] * B[14] + A[14] * B[15];
   B[15] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[15];

   return B;
}

/**

  Equivalent to matrix_multiply(A, rotation_x(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.

 **/

static GLfloat *rotate_x(GLfloat * A, float ang)
{
   GLfloat cosa = cos(ang);
   GLfloat sina = sin(ang);
   GLfloat T[4];

   T[0] = A[4];
   T[1] = A[5];
   T[2] = A[6];
   T[3] = A[7];

   A[4] = T[0] * cosa + A[8] * sina;
   A[5] = T[1] * cosa + A[9] * sina;
   A[6] = T[2] * cosa + A[10] * sina;
   A[7] = T[3] * cosa + A[11] * sina;

   A[8] = A[8] * cosa - T[0] * sina;
   A[9] = A[9] * cosa - T[1] * sina;
   A[10] = A[10] * cosa - T[2] * sina;
   A[11] = A[11] * cosa - T[3] * sina;

   return A;
}

/**

  Equivalent to matrix_multiply(A, rotation_y(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.

 **/

static GLfloat *rotate_y(GLfloat * A, float ang)
{
   GLfloat cosa = cos(ang);
   GLfloat sina = sin(ang);
   GLfloat T[4];

   T[0] = A[0];
   T[1] = A[1];
   T[2] = A[2];
   T[3] = A[3];

   A[0] = T[0] * cosa - A[8] * sina;
   A[1] = T[1] * cosa - A[9] * sina;
   A[2] = T[2] * cosa - A[10] * sina;
   A[3] = T[3] * cosa - A[11] * sina;

   A[8] = T[0] * sina + A[8] * cosa;
   A[9] = T[1] * sina + A[9] * cosa;
   A[10] = T[2] * sina + A[10] * cosa;
   A[11] = T[3] * sina + A[11] * cosa;

   return A;
}

/**

  Equivalent to matrix_multiply(A, rotation_z(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.

 **/

static GLfloat *rotate_z(GLfloat * A, float ang)
{
   GLfloat cosa = cos(ang);
   GLfloat sina = sin(ang);
   GLfloat T[4];

   T[0] = A[0];
   T[1] = A[1];
   T[2] = A[2];
   T[3] = A[3];

   A[0] = T[0] * cosa + A[4] * sina;
   A[1] = T[1] * cosa + A[5] * sina;
   A[2] = T[2] * cosa + A[6] * sina;
   A[3] = T[3] * cosa + A[7] * sina;

   A[4] = A[4] * cosa - T[0] * sina;
   A[5] = A[5] * cosa - T[1] * sina;
   A[6] = A[6] * cosa - T[2] * sina;
   A[7] = A[7] * cosa - T[3] * sina;

   return A;
}

/**

  Creates a translation matrix as per OpenGL definition of Translatef.

 **/

static GLfloat *translation(GLfloat * M, float x, float y, float z)
{
   M[0] = Identity[0];
   M[1] = Identity[1];
   M[2] = Identity[2];
   M[3] = Identity[3];
   M[4] = Identity[4];
   M[5] = Identity[5];
   M[6] = Identity[6];
   M[7] = Identity[7];
   M[8] = Identity[8];
   M[9] = Identity[9];
   M[10] = Identity[10];
   M[11] = Identity[11];
   M[12] = x;
   M[13] = y;
   M[14] = z;
   M[15] = 1.0f;
   return M;
}

/**

  Creates a view matrix that represents the camera's position and orientation.
  Unlike the modelview matrix, which might change for each draw call, the
  view matrix can only have one value per iteration. In this demo, the view matrix
  only changes when the user hits the arrow keys or the 'z' and 'Z' keys.

 **/

static void viewmatrix(void)
{
   translation(ViewMatrix, 0.0f, 0.0f, -40.0f);
   rotate_x(ViewMatrix, view_rotx);
   rotate_y(ViewMatrix, view_roty);
   rotate_z(ViewMatrix, view_rotz);
}

/**

  Changes for ES 2.0 compliance:

    * GLEW is used to access OpenGL 1.5 and 2.1 functions in Windows.

    * A program needs to be created to replace the fixed-function pipeline.
      The program has a single vertex shader and a single fragment shader.
      The vertex shader is read from the gears.vs file and the fragment
      shader is read from the gears.fs file.

    * Fixed function lighting is not supported as per ES 2.0 spec para 2.14.
      The enabling of GL_LIGHTING and GL_LIGHT0 is not necessary.
      The light position is store in a user-defined uniform in the vertex shader
      instead of using the pre-defined gl_LightSource[0]'s position member.

    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      Immediate mode is not supported as per ES 2.0 spec para 2.6.
      Two buffers are created to hold the vertex data and indices respectively.

    * ES 2.0 only supports generic vertex attributes as per spec para 2.6.

 **/

static void gears_init(int width, int height)
{
   static const float pos[3] = { 0.4082f, 0.4082f, 0.8165f };

   GLfloat h = (GLfloat) height / (GLfloat) width;
   GLuint pos_index = 0, norm_index = 1;
   GLintptr offset;

   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);

   load_program();
   pos_index = glGetAttribLocation(program, "Vertex");
   norm_index = glGetAttribLocation(program, "Normal");
   glUniform3fv(light_pos_loc, 1, pos);

   gear(1.0, 4.0, 1.0, 20, 0.7, 0, &gear1);
   gear(0.5, 2.0, 2.0, 10, 0.7, gear1.vcount, &gear2);
   gear(1.3, 2.0, 0.5, 10, 0.7, gear1.vcount + gear2.vcount, &gear3);

   glGenBuffers(2, buffers);
   glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

   glBufferData(GL_ARRAY_BUFFER,
           (gear1.vcount + gear2.vcount +
            gear3.vcount) * sizeof(struct vertex), NULL,
           GL_STATIC_DRAW);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER,
           (gear1.icount + gear2.icount +
            gear3.icount) * sizeof(GLushort), NULL, GL_STATIC_DRAW);

   offset = 0;
   glBufferSubData(GL_ARRAY_BUFFER, offset,
         gear1.vcount * sizeof(struct vertex), gear1.vertices);
   offset += gear1.vcount * sizeof(struct vertex);
   glBufferSubData(GL_ARRAY_BUFFER, offset,
         gear2.vcount * sizeof(struct vertex), gear2.vertices);
   offset += gear2.vcount * sizeof(struct vertex);
   glBufferSubData(GL_ARRAY_BUFFER, offset,
         gear3.vcount * sizeof(struct vertex), gear3.vertices);

   offset = 0;
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset,
         gear1.icount * sizeof(GLushort), gear1.indices);
   offset += gear1.icount * sizeof(GLushort);
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset,
         gear2.icount * sizeof(GLushort), gear2.indices);
   offset += gear2.icount * sizeof(GLushort);
   glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset,
         gear3.icount * sizeof(GLushort), gear3.indices);

   free(gear1.vertices);
   free(gear1.indices);
   free(gear2.vertices);
   free(gear2.indices);
   free(gear3.vertices);
   free(gear3.indices);

   glEnableVertexAttribArray(pos_index);
   glVertexAttribPointer(pos_index, 3, GL_FLOAT, GL_FALSE,
               sizeof(struct vertex), (const GLvoid *)0);
   glEnableVertexAttribArray(norm_index);
   glVertexAttribPointer(norm_index, 3, GL_FLOAT, GL_FALSE,
               sizeof(struct vertex),
               (const GLvoid *)(3 * sizeof(float)));

   glViewport(0, 0, (GLint) width, (GLint) height);

   frustum(-1.0f, 1.0f, -h, h, 5.0f, 60.0f);
}

/**

  Changes for ES 2.0 compliance:

    * Display lists are not supported as per ES 2.0 spec paras 2.4 and 5.4.
      They were replaced by DrawElements with the data and indices sourced from VBOs.
      DrawRangeElements is not supported as per ES 2.0 spec para 2.8.
      The material calls saved in the original display lists must be performed
      before each DrawElements, since each gear has a different color.

    * Fixed function lighting is not supported as per ES 2.0 spec para 2.14.
      The material calls have been replaced by a single user-defined uniform.
      The product of the light model's ambient value with the material is
      pre-computed and stored in a uniform to replace gl_FrontLightModelProduct's
      ambient component.

    * Alpha is omitted from the materials because neither GL_ALPHA_TEST or
      GL_BLEND is enabled by default.

    * The fixed function transformation pipeline is not supported as per ES 2.0 spec para 2.11.
      The matrix operations were replaced by a local implementation.
      The stack approach was not used because it is not efficient.
      The ModelViewProjectionMatrix and NormalMatrix uniforms replace the pre-defined
      gl_ModelViewProjectionMatrix and gl_NormalMatrix uniforms. The other pre-defined
      matrices are not calculated since they are not used by the shaders.

 **/

static void gears_draw(int rotation, int useAlternateColors)
{
   static const GLfloat lmred[4] = { 0.16f, 0.02f, 0.0f, 1.0f };
   static const GLfloat lmgreen[4] = { 0.0f, 0.16f, 0.04f, 1.0f };
   static const GLfloat lmblue[4] = { 0.04f, 0.04f, 0.2f, 1.0f };
   static const GLfloat lmwhite[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

   static const GLfloat red[4] = { 0.8f, 0.1f, 0.0f, 1.0f };
   static const GLfloat green[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
   static const GLfloat blue[4] = { 0.2f, 0.2f, 1.0f, 1.0f };
   static const GLfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };


   viewmatrix();

   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   translation(ModelViewProjectionMatrix, -3.0, -2.0, 0.0);
   matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
   rotate_z(ModelViewProjectionMatrix, angle);
   matrix_copy_3x3((float *)NormalMatrix, ModelViewProjectionMatrix);
   matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);

#if 0
   ModelViewProjectionMatrix[0] = 5.0f;
   ModelViewProjectionMatrix[1] = 0.0f;
   ModelViewProjectionMatrix[2] = 0.0f;
   ModelViewProjectionMatrix[3] = 0.0f;
   ModelViewProjectionMatrix[4] = 0.0f;
   ModelViewProjectionMatrix[5] = 6.25f;
   ModelViewProjectionMatrix[6] = 0.0;
   ModelViewProjectionMatrix[7] = 0.0f;
   ModelViewProjectionMatrix[8] = 0.0f;
   ModelViewProjectionMatrix[9] = 0.0f;
   ModelViewProjectionMatrix[10] = -1.18182;
   ModelViewProjectionMatrix[11] = -1.0f;
   ModelViewProjectionMatrix[12] = 0.0f;
   ModelViewProjectionMatrix[13] = 0.0f;
   ModelViewProjectionMatrix[14] = -10.9091f;
   ModelViewProjectionMatrix[15] = 0.0f;
#endif

   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE,
            (GLfloat *) ModelViewProjectionMatrix);
   glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE,
            (GLfloat *) NormalMatrix);

   if(useAlternateColors)
   {
       glUniform3fv(ambient_diffuse_loc, 1, (GLfloat *) & white);
       glUniform3fv(lm_prod_loc, 1, (GLfloat *) lmwhite);
   }
   else
   {
       glUniform3fv(ambient_diffuse_loc, 1, (GLfloat *) & red);
       glUniform3fv(lm_prod_loc, 1, (GLfloat *) lmred);
   }

   glDrawElements(GL_TRIANGLES, gear1.icount, GL_UNSIGNED_SHORT,
             (const GLvoid *)0);

   translation(ModelViewProjectionMatrix, 3.1, -2.0, 0.0);
   matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
   rotate_z(ModelViewProjectionMatrix, -2.0 * angle - 9.0 * deg2rad);
   matrix_copy_3x3((float *)NormalMatrix, ModelViewProjectionMatrix);
   matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);

   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE,
            (GLfloat *) ModelViewProjectionMatrix);
   glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE,
            (GLfloat *) NormalMatrix);
   glUniform3fv(ambient_diffuse_loc, 1, (GLfloat *) & green);
   glUniform3fv(lm_prod_loc, 1, (GLfloat *) lmgreen);

   glDrawElements(GL_TRIANGLES, gear2.icount, GL_UNSIGNED_SHORT,
             (const GLvoid *)(gear1.icount * sizeof(GLushort)));

   translation(ModelViewProjectionMatrix, -3.1, 4.2, 0.0);
   matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
   rotate_z(ModelViewProjectionMatrix, -2.0 * angle - 25.0 * deg2rad);
   matrix_copy_3x3((float *)NormalMatrix, ModelViewProjectionMatrix);
   matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);

   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE,
            (GLfloat *) ModelViewProjectionMatrix);
   glUniformMatrix3fv(normal_matrix_loc, 1, GL_FALSE,
            (GLfloat *) NormalMatrix);
   glUniform3fv(ambient_diffuse_loc, 1, (GLfloat *) & blue);
   glUniform3fv(lm_prod_loc, 1, (GLfloat *) lmblue);

   glDrawElements(GL_TRIANGLES, gear3.icount, GL_UNSIGNED_SHORT,
             (const GLvoid *)((gear1.icount + gear2.icount) *
               sizeof(GLushort)));

   switch (rotation) {
   case 0:
   case 6:
      angle += 2.0f * deg2rad;
      view_rotx += 1.0f * deg2rad;
      break;
   case 1:
   case 7:
      angle += 2.0f * deg2rad;
      view_roty += 1.0f * deg2rad;
      break;
   case 2:
      angle += 2.0f * deg2rad;
      view_rotz += 1.0f * deg2rad;
      break;
   case 3:
      angle -= 2.0f * deg2rad;
      view_rotx += 1.0f * deg2rad;
      break;
   case 4:
      angle -= 2.0f * deg2rad;
      view_roty += 1.0f * deg2rad;
      break;
   case 5:
      angle -= 2.0f * deg2rad;
      view_rotz += 1.0f * deg2rad;
      break;
   }
}

static void gears_resize(GLint width, GLint height)
{
   GLfloat h = (GLfloat) height / (GLfloat) width;

   glViewport(0, 0, (GLint) width, (GLint) height);

   frustum(-1.0f, 1.0f, -h, h, 5.0f, 60.0f);
}

/*----------------------------------------------------------------------------------------------*/

static int running = 1;

static void
signal_int(int signum)
{
   UNUSED(signum);
   running = 0;
}

static void
print_usage(const char* prog)
{
   printf("Usage: %s [OPTIONS...]\n", prog);
   puts("\t-g|--geometry <width>x<height>");
   puts("\t-i|--interval <swap-interval>");
   puts("\t-I|--ivi-id <surface-id>");
   puts("\t-q|--quiet");
   puts("\t-s|--still");
   puts("\t-a|--alternateColors");
   puts("\t-t|--sleepTimeInMillis <ms-to-sleep-each-frame>");
   puts("\t-f|--fps <frame-rate-per-second>");
   puts("\t-h|--help");
}

int main(int argc, char *argv[])
{
   setbuf(stdout, NULL); // Immediately write to stdout without flush

   int startupTime = gettime();

   int quiet = 0;
   int useAlternateColors = 0;
   unsigned int sleepTimeInMillis = 0;
   float fps = 1000.0f;
   int opt;
   static const char opts[] = "g:i:I:L:qsh:at:f:";
   static const struct option longopts[] = {
      { "geometry", 1, NULL, 'g' },
      { "interval", 1, NULL, 'i' },
      { "ivi-id", 1, NULL, 'I' },
      { "layer-id", 0, NULL, 'L' },
      { "quiet", 0, NULL, 'q' },
      { "help", 0, NULL, 'h' },
      { "still", 0, NULL, 's' },
      { "alternateColors", 0, NULL, 'a' },
      { "fps", 1, NULL, 'f' },
      { "sleepTimeInMillis", 1, NULL, 't' },
      { NULL, 0, NULL, 0 }
   };
   int interval = 1;

   window.width = 384;
   window.height = 384;

   int rotation = 0;

   while ((opt = getopt_long(argc, argv, opts, longopts, NULL)) > 0)
   {
      switch (opt) {
      case 'g':
         if (sscanf(optarg, "%dx%d", &window.width, &window.height) != 2)
         {
            fprintf(stderr, "wrong geometry format: %s\n", optarg);
         };
         break;
      case 'i':
         if (sscanf(optarg, "%d", &interval) != 1)
         {
            fprintf(stderr, "invalid swap interval: %s\n", optarg);
         };
         break;
      case 'I':
         if (sscanf(optarg, "%u", &ivi_surface_id) != 1)
         {
            fprintf(stderr, "invalid ivi-id: %s\n", optarg);
         };
         break;
      case 'L':
          if (sscanf(optarg, "%u", &ivi_layer_id) != 1)
          {
             fprintf(stderr, "invalid layer-id: %s\n", optarg);
          };
          break;
      case 'q':
         quiet = 1;
         break;
      case 's':
          rotation = 8;
          break;
      case 'a':
          useAlternateColors = 1;
          break;
      case 'f':
          if (sscanf(optarg, "%f", &fps) != 1)
          {
             fprintf(stderr, "invalid fps: %s\n", optarg);
          }
          else
          {
              sleepTimeInMillis = (unsigned int) (1000.0f/ fps);
          };
          break;
      case 't':
          if (sscanf(optarg, "%u", &sleepTimeInMillis) != 1)
          {
             fprintf(stderr, "invalid sleep time: %s\n", optarg);
          };
          break;
      case 'h':
      default:
         print_usage(argv[0]);
         return opt != 'h';
      }
   }

   if (setupWayland() < 0) {
      fprintf(stderr, "could not setup Wayland\n");
      return 1;
   }

   if (setupEGL(!quiet) < 0) {
      fprintf(stderr, "could not setup EGL context\n");
      return 1;
   }

   if (createWindow() < 0) {
      fprintf(stderr, "could not create window\n");
      return 1;
   }

   eglSwapInterval(egldisplay, interval);

   EGLint h, w;
   eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &w);
   eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &h);

   if (!quiet) printf("Surface dimensions: %ix%i\n", w, h);

   gears_init(w, h);
   gears_resize(w, h);

   int start = gettime();
   int frames = 0;

   signal(SIGINT, &signal_int);

   bool firstFrame = true;

   while (running) {
      const int now = gettime();

      if (!quiet && now - start > 5000) {
         double elapsed = (double)(now - start) / 1000.0;
         printf("%d frames in %3.1f seconds = %6.3f FPS\n",
                frames, elapsed, frames / elapsed);
         start = now;
         frames = 0;
      }
      gears_draw(rotation, useAlternateColors);

      eglSwapBuffers(egldisplay, eglsurface);
      if (firstFrame)
      {
          firstFrame = false;
          int elapsedTimeSinceStartup = gettime() - startupTime;

          // Integration tests are waiting on this message to synchronize until a frame was rendered, so don't change this text
          printf("time since startup until first eglSwapBuffers done: %d ms\n", elapsedTimeSinceStartup);

          // With weston, ivi_controller_commit_changes seems to work only, when a frame was already rendered to
          // the surface, so call make_surface_visible after the eglSwapBuffers.
          make_surface_visible();
      }
      frames++;

      if (check_events() == -1)
         break;

      if(0 != sleepTimeInMillis)
      {
          const int timeAtEnd = gettime();
          const int actualTimeToSleep = sleepTimeInMillis - (timeAtEnd - now);

          if(actualTimeToSleep > 0)
          {
              const struct timespec sleepTime = {actualTimeToSleep / 1000u,  1000000u * (actualTimeToSleep % 1000u)};
              nanosleep(&sleepTime, NULL);
          }
      }
   }

   destroyWindow();
   terminateEGL();
   closeWayland();

   if (!quiet) printf("exit\n");

   return 0;
}
