//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACE_H
#define RAMSES_SURFACE_H

#include "RendererAPI/ISurface.h"

namespace ramses_internal
{
    class IWindow;
    class IContext;

    class Surface final : public ISurface
    {
    public:
        Surface(IWindow& window, IContext& context);
        ~Surface() override;

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
        IWindow& m_window;
        IContext& m_context;
    };
}

#endif
