//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

#include <string>
#include <string_view>

namespace ramses::internal
{
    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        virtual bool init() = 0;
        virtual bool setFullscreen(bool fullscreen) = 0;
        virtual bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) = 0;
        [[nodiscard]] virtual bool canRenderNewFrame() const = 0;
        virtual void handleEvents() = 0;
        virtual void frameRendered() = 0;
        [[nodiscard]] virtual uint32_t getWidth() const = 0;
        [[nodiscard]] virtual uint32_t getHeight() const = 0;
        [[nodiscard]] virtual float getAspectRatio() const = 0;
        [[nodiscard]] virtual int32_t getPosX() const = 0;
        [[nodiscard]] virtual int32_t getPosY() const = 0;
        virtual void setTitle(std::string_view title) = 0;
        [[nodiscard]] virtual const std::string& getTitle() const = 0;
        [[nodiscard]] virtual bool hasTitle() const = 0;
        [[nodiscard]] virtual WaylandIviSurfaceId getWaylandIviSurfaceID() const = 0;
    };
}
