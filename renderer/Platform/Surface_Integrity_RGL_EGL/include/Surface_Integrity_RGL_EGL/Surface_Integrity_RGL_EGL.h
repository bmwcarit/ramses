//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_INTEGRITY_RGL_EGL_H
#define RAMSES_SURFACE_INTEGRITY_RGL_EGL_H

#include "Platform_Base/Surface_Base.h"

namespace ramses_internal
{
    class Window_Integrity_RGL;
    class Context_EGL;

    class Surface_Integrity_RGL_EGL : public Surface_Base
    {
    public:
        Surface_Integrity_RGL_EGL(Window_Integrity_RGL& window, Context_EGL& context);
        ~Surface_Integrity_RGL_EGL() override;

        Bool swapBuffers() final;
        Bool enable() final;
        Bool disable() final;

    private:
        Context_EGL& m_context;
    };
}

#endif
