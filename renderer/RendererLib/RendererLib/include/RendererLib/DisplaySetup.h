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
    struct MappedSceneInfo
    {
        SceneId sceneId;
        Int32 globalSceneOrder;
        Bool shown;
    };
    typedef std::vector<MappedSceneInfo> MappedScenes;

    struct DisplayBufferInfo
    {
        Bool isOffscreenBuffer;
        Bool isInterruptible;
        Viewport viewport;
        Vector4 clearColor;
        MappedScenes mappedScenes;
        Bool needsRerender;
    };
    using DisplayBuffersMap = std::map<DeviceResourceHandle, DisplayBufferInfo>;

    class DisplaySetup
    {
    public:
        void registerDisplayBuffer(DeviceResourceHandle displayBuffer, const Viewport& viewport, const Vector4& clearColor, Bool isOffscreenBuffer, Bool isInterruptible);
        void unregisterDisplayBuffer(DeviceResourceHandle displayBuffer);

        const DisplayBufferInfo& getDisplayBuffer(DeviceResourceHandle displayBuffer) const;

        void setDisplayBufferToBeRerendered(DeviceResourceHandle displayBuffer, Bool rerender);

        void                 mapSceneToDisplayBuffer(SceneId sceneId, DeviceResourceHandle displayBuffer, Int32 sceneOrder);
        void                 unmapScene(SceneId sceneId);
        DeviceResourceHandle findDisplayBufferSceneIsMappedTo(SceneId sceneId) const;
        void                 setSceneShown(SceneId sceneId, Bool show);
        void                 setClearColor(DeviceResourceHandle displayBuffer, const Vector4& clearColor);

        const DeviceHandleVector& getNonInterruptibleOffscreenBuffersToRender() const;
        const DeviceHandleVector& getInterruptibleOffscreenBuffersToRender(DeviceResourceHandle interruptedDisplayBuffer) const;

        const DisplayBuffersMap& getDisplayBuffers() const;

    private:
        DisplayBufferInfo& getDisplayBufferInternal(DeviceResourceHandle displayBuffer);

        DisplayBuffersMap m_displayBuffers;

        // keep as member to avoid re-allocations
        mutable DeviceHandleVector m_buffersToRender;
    };
}

#endif
