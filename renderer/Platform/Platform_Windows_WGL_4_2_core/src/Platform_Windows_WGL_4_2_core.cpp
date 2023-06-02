//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL_4_2_core/Platform_Windows_WGL_4_2_core.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    Platform_Windows_WGL_4_2_core::Platform_Windows_WGL_4_2_core(const RendererConfig& rendererConfig)
        : Platform_Windows_WGL(rendererConfig)
    {
    }
}
