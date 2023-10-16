//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/Window_Wayland.h"
#include "internal/Platform/EGL/Platform_EGL.h"

namespace ramses::internal
{
    class Platform_Wayland_EGL : public Platform_EGL<Window_Wayland>
    {
    protected:
        explicit Platform_Wayland_EGL(const RendererConfig& rendererConfig);
        ~Platform_Wayland_EGL() override;

        bool createEmbeddedCompositor(const DisplayConfig& displayConfig) override;
        void createTextureUploadingAdapter(const DisplayConfig& displayConfig) override;

        [[nodiscard]] uint32_t getSwapInterval() const override;

        //TODO Mohamed: remove use of EC dummy as soon as it is possible to create multiple displays on wayland
        [[nodiscard]] static bool IsCreatingWaylandEmbeddedCompositorRequired(const DisplayConfig& displayConfig);

        const std::chrono::microseconds m_frameCallbackMaxPollTime;
    };
}

