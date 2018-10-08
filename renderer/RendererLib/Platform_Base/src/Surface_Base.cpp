//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/Surface_Base.h"
#include "Platform_Base/Window_Base.h"
#include "Platform_Base/Context_Base.h"

namespace ramses_internal
{
    Surface_Base::Surface_Base(Window_Base& window, Context_Base& context)
        : m_window(window)
        , m_context(context)
    {
    }

    Bool Surface_Base::canRenderNewFrame() const
    {
        return m_window.canRenderNewFrame();
    }

    void Surface_Base::frameRendered()
    {
        m_window.frameRendered();
    }

    const IWindow& Surface_Base::getWindow() const
    {
        return m_window;
    }

    IWindow& Surface_Base::getWindow()
    {
        return m_window;
    }

    const IContext& Surface_Base::getContext() const
    {
        return m_context;
    }

    IContext& Surface_Base::getContext()
    {
        return m_context;
    }
}

