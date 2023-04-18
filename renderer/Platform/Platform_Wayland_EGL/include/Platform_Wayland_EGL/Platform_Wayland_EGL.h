//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_WAYLAND_EGL_H
#define RAMSES_PLATFORM_WAYLAND_EGL_H

#include "Window_Wayland/Window_Wayland.h"
#include "Platform_EGL/Platform_EGL.h"

namespace ramses_internal
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
        [[nodiscard]] Bool isCreatingWaylandEmbeddedCompositorRequired(const DisplayConfig& displayConfig) const;

        const std::chrono::microseconds m_frameCallbackMaxPollTime;
    };
}

#endif

