//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_EGL_OFFSCREEN_H
#define RAMSES_SURFACE_EGL_OFFSCREEN_H

#include "Platform_Base/Surface_Base.h"

namespace ramses_internal
{
    class Window_Wayland;
    class Context_EGL;

    class Surface_EGL_Offscreen : public Surface_Base
    {
    public:
        Surface_EGL_Offscreen(Window_Wayland& window, Context_EGL& context);
        ~Surface_EGL_Offscreen() override;

        void frameRendered() override final;
        Bool canRenderNewFrame() const override final;
        Bool swapBuffers() override final;
        Bool enable() override final;
        Bool disable() override final;

    private:
        Context_EGL& m_context;
    };
}

#endif

