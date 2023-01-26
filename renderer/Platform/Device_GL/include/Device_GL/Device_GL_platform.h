//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICE_GL_PLATFORM_H
#define RAMSES_DEVICE_GL_PLATFORM_H

#ifdef _WIN32
#include "PlatformAbstraction/MinimalWindowsH.h"
#include <GL/gl.h>
#include <GL/glext.h>

#elif defined(__linux__)
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
// deprecated
#include <GLES3/gl3ext.h>
// should always include that file as well, since gl3ext doesn't define ANY extensions, see http://www.khronos.org/registry/gles/#headers
#include <GLES2/gl2ext.h>

#elif defined(__ghs__)
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
// deprecated
#include <GLES3/gl3ext_REL.h>
// should always include that file as well, since gl3ext doesn't define ANY extensions, see http://www.khronos.org/registry/gles/#headers
#include <GLES2/gl2ext.h>
#endif

#if defined(__linux__) || defined (__ghs__)
    #include "Device_GL/Device_GL_platform_linux.h"
#endif // LINUX

#ifdef _WIN32
    #include "Device_GL/Device_GL_platform_windows.h"
#endif // WIN32

namespace ramses_internal
{
    // OpenGL API
    DECLARE_ALL_API_PROCS
}

// TODO Violin fix OpenGL headers properly...
// This is so that the external texture sampler (not texture) can be used in scene also on platforms not actually supporting external texture
// (e.g. desktop windows when creating assets to be used with external texture).
#if !defined(GL_OES_EGL_image_external)
#define GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

#endif
