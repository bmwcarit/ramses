//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WINDOWS_WGL_ES_3_0_H
#define RAMSES_PLATFORM_WINDOWS_WGL_ES_3_0_H

#define DEVICE_TYPE_ID EDeviceTypeId_GL_ES_3_0

#include "Platform_Windows_WGL/Platform_Windows_WGL.h"


namespace ramses_internal
{
    class Platform_Windows_WGL_ES_3_0 : public Platform_Windows_WGL
    {
    public:
        Platform_Windows_WGL_ES_3_0(const RendererConfig& rendererConfig);

        virtual bool createDevice() final;
        virtual bool createDeviceUploading() final;

        virtual const Int32* getContextAttributes() final;
    };
}

#endif
