//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/Surface.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IContext.h"

namespace ramses_internal
{
    Surface::Surface(IWindow& window, IContext& context)
        : m_window(window)
        , m_context(context)
    {
    }

    Surface::~Surface()
    {
        disable();
    }

    Bool Surface::canRenderNewFrame() const
    {
        return m_window.canRenderNewFrame();
    }

    void Surface::frameRendered()
    {
        m_window.frameRendered();
    }

    const IWindow& Surface::getWindow() const
    {
        return m_window;
    }

    IWindow& Surface::getWindow()
    {
        return m_window;
    }

    const IContext& Surface::getContext() const
    {
        return m_context;
    }

    IContext& Surface::getContext()
    {
        return m_context;
    }

    Bool Surface::swapBuffers()
    {
        return m_context.swapBuffers();
    }

    Bool Surface::enable()
    {
        return m_context.enable();
    }

    Bool Surface::disable()
    {
        return m_context.disable();
    }
}

