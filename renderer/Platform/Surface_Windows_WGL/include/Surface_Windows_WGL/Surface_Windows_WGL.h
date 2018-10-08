//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_WINDOWS_WGL_H
#define RAMSES_SURFACE_WINDOWS_WGL_H

#include "Platform_Base/Surface_Base.h"

namespace ramses_internal
{
    class Window_Windows;
    class Context_WGL;

    class Surface_Windows_WGL : public Surface_Base
    {
    public:
        Surface_Windows_WGL(Window_Windows& window, Context_WGL& context);
        ~Surface_Windows_WGL() override;

        Bool swapBuffers() final;

        Bool enable() final;
        Bool disable() final;
    private:
        Window_Windows& m_window;
        Context_WGL& m_context;
    };
}

#endif
