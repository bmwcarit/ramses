//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_BASE_H
#define RAMSES_SURFACE_BASE_H

#include "RendererAPI/ISurface.h"


namespace ramses_internal
{
    class Window_Base;
    class Context_Base;

    class Surface_Base : public ISurface
    {
    public:
        Surface_Base(Window_Base& window, Context_Base& context);
        ~Surface_Base() override;

        Bool canRenderNewFrame() const override;
        void frameRendered() override;

        const IWindow& getWindow() const override final;
        IWindow& getWindow() override final;
        const IContext& getContext() const override final;
        IContext& getContext() override final;

        bool swapBuffers() override;
        bool enable() override final;
        bool disable() override final;

    protected:
        Window_Base& m_window;
        Context_Base& m_context;
    };
}

#endif
