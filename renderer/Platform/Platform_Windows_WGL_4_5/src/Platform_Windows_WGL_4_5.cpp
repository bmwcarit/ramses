//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Windows_WGL_4_5/Platform_Windows_WGL_4_5.h"

#include "Context_WGL/Context_WGL.h"
#include "Device_GL/Device_GL.h"

#include "Utils/LogMacros.h"


namespace ramses_internal
{
    IPlatform* Platform_Base::CreatePlatform(const RendererConfig& rendererConfig)
    {
        return new Platform_Windows_WGL_4_5(rendererConfig);
    }

    Platform_Windows_WGL_4_5::Platform_Windows_WGL_4_5(const RendererConfig& rendererConfig)
        : Platform_Windows_WGL(rendererConfig)
    {
    }

    IDevice* Platform_Windows_WGL_4_5::createDevice(IContext& context)
    {
        Context_WGL* platformContext = getPlatformContext<Context_WGL>(context);
        assert(0 != platformContext);
        Device_GL* device = new Device_GL(*platformContext, 4, 5, false);
        return addPlatformDevice(device);
    }

    const Int32* Platform_Windows_WGL_4_5::getContextAttributes()
    {
        const Int32* returnValue = NULL;

        if(m_wglExtensions.isExtensionAvailable("create_context_profile"))
        {
            static const Int32 attribs[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 5,
                //WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0
            };
            returnValue = attribs;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Windows_WGL_4_5::getContextAttributes:  could not load WGL context attributes");
        }

        return returnValue;
    }
}
