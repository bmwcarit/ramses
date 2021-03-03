//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYSETUP_H
#define RAMSES_DISPLAYSETUP_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/Viewport.h"
#include "Math3d/Vector4.h"
#include "Collections/Vector.h"
#include <map>

namespace ramses_internal
{
    struct AssignedSceneInfo
    {
        SceneId sceneId;
        Int32 globalSceneOrder;
        Bool shown;
    };
    using AssignedScenes = std::vector<AssignedSceneInfo>;

    struct DisplayBufferInfo
    {
        bool isOffscreenBuffer;
        bool isInterruptible;
        Viewport viewport;
        uint32_t clearFlags;
        Vector4 clearColor;
        AssignedScenes scenes;
        bool needsRerender;
    };
    using DisplayBuffersMap = std::map<DeviceResourceHandle, DisplayBufferInfo>;

    class DisplaySetup
    {
    public:
        void registerDisplayBuffer(DeviceResourceHandle displayBuffer, const Viewport& viewport, const Vector4& clearColor, Bool isOffscreenBuffer, Bool isInterruptible);
        void unregisterDisplayBuffer(DeviceResourceHandle displayBuffer);

        const DisplayBufferInfo& getDisplayBuffer(DeviceResourceHandle displayBuffer) const;

        void setDisplayBufferToBeRerendered(DeviceResourceHandle displayBuffer, Bool rerender);

        void                 assignSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle displayBuffer, Int32 sceneOrder);
        void                 unassignScene(SceneId sceneId);
        DeviceResourceHandle findDisplayBufferSceneIsAssignedTo(SceneId sceneId) const;
        void                 setSceneShown(SceneId sceneId, Bool show);
        void                 setClearFlags(DeviceResourceHandle displayBuffer, uint32_t clearFlags);
        void                 setClearColor(DeviceResourceHandle displayBuffer, const Vector4& clearColor);

        const DeviceHandleVector& getNonInterruptibleOffscreenBuffersToRender() const;
        const DeviceHandleVector& getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle interruptedDisplayBuffer) const;

        const DisplayBuffersMap& getDisplayBuffers() const;

    private:
        AssignedSceneInfo& findSceneInfo(SceneId sceneId, DeviceResourceHandle displayBuffer);
        DisplayBufferInfo& getDisplayBufferInternal(DeviceResourceHandle displayBuffer);

        DisplayBuffersMap m_displayBuffers;

        // keep as member to avoid re-allocations
        mutable DeviceHandleVector m_buffersToRender;
    };
}

#endif
