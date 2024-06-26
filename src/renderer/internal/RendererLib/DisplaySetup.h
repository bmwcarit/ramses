//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/Viewport.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "impl/DataTypesImpl.h"
#include <map>

namespace ramses::internal
{
    struct AssignedSceneInfo
    {
        SceneId sceneId;
        int32_t globalSceneOrder{0};
        bool    shown{false};
    };
    using AssignedScenes = std::vector<AssignedSceneInfo>;

    struct DisplayBufferInfo
    {
        bool           isOffscreenBuffer{false};
        bool           isInterruptible{false};
        uint32_t       sampleCount{0u};
        Viewport       viewport;
        ClearFlags     clearFlags;
        glm::vec4      clearColor;
        AssignedScenes scenes;
        bool           needsRerender{false};
    };
    using DisplayBuffersMap = std::map<DeviceResourceHandle, DisplayBufferInfo>;

    class DisplaySetup
    {
    public:
        void registerDisplayBuffer(DeviceResourceHandle displayBuffer, const Viewport& viewport, const glm::vec4& clearColor, bool isOffscreenBuffer, uint32_t sampleCount, bool isInterruptible);
        void unregisterDisplayBuffer(DeviceResourceHandle displayBuffer);

        [[nodiscard]] const DisplayBufferInfo& getDisplayBuffer(DeviceResourceHandle displayBuffer) const;

        void setDisplayBufferToBeRerendered(DeviceResourceHandle displayBuffer, bool rerender);

        void                 assignSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle displayBuffer, int32_t sceneOrder);
        void                 unassignScene(SceneId sceneId);
        [[nodiscard]] DeviceResourceHandle findDisplayBufferSceneIsAssignedTo(SceneId sceneId) const;
        void                 setSceneShown(SceneId sceneId, bool show);
        void                 setClearFlags(DeviceResourceHandle displayBuffer, ClearFlags clearFlags);
        void                 setClearColor(DeviceResourceHandle displayBuffer, const glm::vec4& clearColor);
        void                 setDisplayBufferSize(DeviceResourceHandle displayBuffer, uint32_t width, uint32_t height);

        [[nodiscard]] const DeviceHandleVector& getNonInterruptibleOffscreenBuffersToRender() const;
        [[nodiscard]] const DeviceHandleVector& getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle interruptedDisplayBuffer) const;

        [[nodiscard]] const DisplayBuffersMap& getDisplayBuffers() const;

    private:
        AssignedSceneInfo& findSceneInfo(SceneId sceneId, DeviceResourceHandle displayBuffer);
        DisplayBufferInfo& getDisplayBufferInternal(DeviceResourceHandle displayBuffer);

        DisplayBuffersMap m_displayBuffers;

        // keep as member to avoid re-allocations
        mutable DeviceHandleVector m_buffersToRender;
    };
}
