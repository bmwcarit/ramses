//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_EGL_H
#define RAMSES_PLATFORM_EGL_H

#include "Platform_Base/Platform_Base.h"
#include "RendererLib/RendererConfig.h"
#include "Context_EGL/Context_EGL.h"
#include "Device_GL/Device_GL.h"

namespace ramses_internal
{
    template <typename WindowT>
    class Platform_EGL : public Platform_Base
    {
    protected:
        explicit Platform_EGL(const RendererConfig& rendererConfig)
            : Platform_Base(rendererConfig)
        {
        }

        virtual IContext* createContext(IWindow& window, IContext* sharedContext) override final
        {
            WindowT* platformWindow = getPlatformWindow<WindowT>(window);
            assert(nullptr != platformWindow);

            const auto swapInterval = getSwapInterval();
            const std::vector<EGLint> contextAttributes = getContextAttributes();
            const std::vector<EGLint> surfaceAttributes = getSurfaceAttributes(platformWindow->getMSAASampleCount());
            Context_EGL* platformSharedContext = nullptr;

            if(sharedContext)
            {
                platformSharedContext = getPlatformContext<Context_EGL>(*sharedContext);
                assert(platformSharedContext);
            }


            Context_EGL* platformContext = new Context_EGL(
                        platformWindow->getNativeDisplayHandle(),
                        reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                        contextAttributes.data(),
                        surfaceAttributes.data(),
                        nullptr,
                        swapInterval,
                        platformSharedContext);

            return addPlatformContext(platformContext);
        }

        virtual IDevice* createDevice(IContext& context) override final
        {
            Context_EGL* platformContext = getPlatformContext<Context_EGL>(context);
            assert(nullptr != platformContext);
            Device_GL* device = new Device_GL(*platformContext, 3, 0, true);
            return addPlatformDevice(device);
        }

        std::vector<EGLint> getContextAttributes() const
        {
            return {
                EGL_CONTEXT_CLIENT_VERSION,
                3,

                EGL_NONE
            };
        }

        virtual uint32_t getSwapInterval() const = 0;
        virtual std::vector<EGLint> getSurfaceAttributes(UInt32 msaaSampleCount) const = 0;
    };
}

#endif
