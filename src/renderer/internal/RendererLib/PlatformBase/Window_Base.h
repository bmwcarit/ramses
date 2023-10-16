//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IWindow.h"

namespace ramses::internal
{
    class DisplayConfig;
    class IWindowEventHandler;

    class Window_Base : public IWindow
    {
    public:
        Window_Base(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, uint32_t id);

        [[nodiscard]] bool canRenderNewFrame() const override;
        void frameRendered() override;
        [[nodiscard]] int32_t getPosX() const final override;
        [[nodiscard]] int32_t getPosY() const final override;

        [[nodiscard]] uint32_t getWidth() const final override;
        [[nodiscard]] uint32_t getHeight() const final override;
        [[nodiscard]] float  getAspectRatio() const final override;

        [[nodiscard]] const std::string& getTitle() const final override;
        void setTitle(std::string_view title) override;

        [[nodiscard]] uint32_t getMSAASampleCount() const;

        [[nodiscard]] WaylandIviSurfaceId getWaylandIviSurfaceID() const final override;

        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    protected:
        std::string m_windowName;
        IWindowEventHandler& m_eventHandler;
        const bool m_fullscreen;
        uint32_t m_msaaSampleCount;

        uint32_t m_width;
        uint32_t m_height;
        int32_t m_posX;
        int32_t m_posY;

        const WaylandIviSurfaceId m_waylandIviSurfaceID;
        const bool m_resizable;
    };
}
