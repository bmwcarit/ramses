//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL/Platform_Windows_WGL.h"

namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Windows_WGL(rendererConfig, EDeviceTypeId_GL_ES_3_0);
    }
}
