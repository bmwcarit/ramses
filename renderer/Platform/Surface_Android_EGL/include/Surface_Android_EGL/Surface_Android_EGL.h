//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_ANDROID_EGL_H
#define RAMSES_SURFACE_ANDROID_EGL_H

#include "Platform_Base/Surface_Base.h"

namespace ramses_internal
{
    class Window_Android;
    class Context_EGL;

    class Surface_Android_EGL : public Surface_Base
    {
    public:
        Surface_Android_EGL(Window_Android& window, Context_EGL& context);
        ~Surface_Android_EGL() final;

        Bool swapBuffers() override final;
        Bool enable() override final;
        Bool disable() override final;
    private:

        Context_EGL& m_context;
    };
}

#endif
