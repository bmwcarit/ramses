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

        virtual IContext* createContext(IWindow& window) override final
        {
            WindowT* platformWindow = getPlatformWindow<WindowT>(window);
            assert(nullptr != platformWindow);

            std::vector<EGLint> contextAttributes;
            getContextAttributes(contextAttributes);
            std::vector<EGLint> surfaceAttributes;
            getSurfaceAttributes(platformWindow->getMSAASampleCount(), surfaceAttributes);

            Context_EGL* platformContext = new Context_EGL(
                        platformWindow->getNativeDisplayHandle(),
                        reinterpret_cast<Context_EGL::Generic_EGLNativeWindowType>(platformWindow->getNativeWindowHandle()),
                        &contextAttributes[0],
                        &surfaceAttributes[0],
                        nullptr,
                        1,
                        nullptr);

            return addPlatformContext(platformContext);
        }

        virtual void getContextAttributes(std::vector<EGLint>& attributes) const = 0;
        virtual void getSurfaceAttributes(UInt32 msaaSampleCount, std::vector<EGLint>& attributes) const = 0;
    };
}

#endif
