//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

/*
 * glad/gles2.h provides the OpenGLES 3.2 API including the extensions that RAMSES uses.
 * The file is generated - see inline documentation for configuration parameters.
 * gladLoadGLES2() needs to be called after context creation to initialize the function pointers
 *
 * This is also expected to work if the context is a GL context >= 3.2 (instead of a GLES context)
 * However, this won't make the context GLES3 compatible by itself. It still requires an extension
 * to support GLES3 shaders (e.g. GL_ARB_ES3_2_compatibility)
 */
#include <glad/gles2.h>

