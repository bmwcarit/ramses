//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WINDOW_BASE_H
#define RAMSES_WINDOW_BASE_H

#include "RendererAPI/IWindow.h"

namespace ramses_internal
{
    class DisplayConfig;
    class IWindowEventHandler;

    class Window_Base : public IWindow
    {
    public:
        Window_Base(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler, UInt32 id);

        [[nodiscard]] bool canRenderNewFrame() const override;
        void frameRendered() override;
        [[nodiscard]] Int32 getPosX() const final override;
        [[nodiscard]] Int32 getPosY() const final override;

        [[nodiscard]] UInt32 getWidth() const final override;
        [[nodiscard]] UInt32 getHeight() const final override;
        [[nodiscard]] float  getAspectRatio() const final override;

        [[nodiscard]] const std::string& getTitle() const final override;
        void setTitle(std::string_view title) override;

        [[nodiscard]] UInt32 getMSAASampleCount() const;

        [[nodiscard]] WaylandIviSurfaceId getWaylandIviSurfaceID() const final override;

        bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;

    protected:
        std::string m_windowName;
        IWindowEventHandler& m_eventHandler;
        const bool m_fullscreen;
        const bool m_borderless;
        UInt32 m_msaaSampleCount;

        UInt32 m_width;
        UInt32 m_height;
        Int32 m_posX;
        Int32 m_posY;

        const WaylandIviSurfaceId m_waylandIviSurfaceID;
        const bool m_resizable;
    };
}

#endif
