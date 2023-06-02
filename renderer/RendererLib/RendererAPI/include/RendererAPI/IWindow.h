//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IWINDOW_H
#define RAMSES_IWINDOW_H

#include "Types.h"

#include <string>
#include <string_view>

namespace ramses_internal
{
    class IWindow
    {
    public:
        virtual ~IWindow(){}

        virtual bool init() = 0;
        virtual bool setFullscreen(bool fullscreen) = 0;
        virtual bool setExternallyOwnedWindowSize(uint32_t width, uint32_t height) = 0;
        [[nodiscard]] virtual bool canRenderNewFrame() const = 0;
        virtual void handleEvents() = 0;
        virtual void frameRendered() = 0;
        [[nodiscard]] virtual UInt32 getWidth() const = 0;
        [[nodiscard]] virtual UInt32 getHeight() const = 0;
        [[nodiscard]] virtual float getAspectRatio() const = 0;
        [[nodiscard]] virtual Int32 getPosX() const = 0;
        [[nodiscard]] virtual Int32 getPosY() const = 0;
        virtual void setTitle(std::string_view title) = 0;
        [[nodiscard]] virtual const std::string& getTitle() const = 0;
        [[nodiscard]] virtual bool hasTitle() const = 0;
        [[nodiscard]] virtual WaylandIviSurfaceId getWaylandIviSurfaceID() const = 0;
    };
}

#endif
