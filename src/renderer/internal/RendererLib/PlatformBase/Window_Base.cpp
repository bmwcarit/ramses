//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/Window_Base.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/RendererLib/PlatformInterface/IWindowEventHandler.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    Window_Base::Window_Base(const DisplayConfigData& displayConfig, IWindowEventHandler& eventHandler, uint32_t id)
        : m_windowName(displayConfig.getWindowTitle().empty() ? fmt::format("RAMSES Renderer {}", id) : displayConfig.getWindowTitle())
        , m_eventHandler(eventHandler)
        , m_fullscreen(displayConfig.getFullscreenState())
        , m_msaaSampleCount(1)
        , m_width(displayConfig.getDesiredWindowWidth())
        , m_height(displayConfig.getDesiredWindowHeight())
        , m_posX(displayConfig.getWindowPositionX())
        , m_posY(displayConfig.getWindowPositionY())
        , m_waylandIviSurfaceID(displayConfig.getWaylandIviSurfaceID())
        , m_resizable(displayConfig.isResizable())
    {
        m_msaaSampleCount = displayConfig.getAntialiasingSampleCount();
    }

    bool Window_Base::canRenderNewFrame() const
    {
        return true;
    }

    void Window_Base::frameRendered()
    {
    }

    const std::string& Window_Base::getTitle() const
    {
        return m_windowName;
    }

    void Window_Base::setTitle(std::string_view title)
    {
        m_windowName = title;
    }

    uint32_t Window_Base::getMSAASampleCount() const
    {
        return m_msaaSampleCount;
    }

    WaylandIviSurfaceId Window_Base::getWaylandIviSurfaceID() const
    {
        return m_waylandIviSurfaceID;
    }

    bool Window_Base::setExternallyOwnedWindowSize(uint32_t /*width*/, uint32_t /*height*/)
    {
        LOG_ERROR(CONTEXT_RENDERER, "Window_Base::setExternallyOwnedWindowSize: platform does not support externally owned windows!");
        return false;
    }

    uint32_t Window_Base::getWidth() const
    {
        return m_width;
    }

    uint32_t Window_Base::getHeight() const
    {
        return m_height;
    }

    float Window_Base::getAspectRatio() const
    {
        return static_cast<float>(m_width) / static_cast<float>(m_height);
    }

    int32_t Window_Base::getPosX() const
    {
        return m_posX;
    }

    int32_t Window_Base::getPosY() const
    {
        return m_posY;
    }
}
