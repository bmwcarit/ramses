//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENTCOLLECTOR_H
#define RAMSES_RENDEREREVENTCOLLECTOR_H

#include "RendererLib/RendererEvent.h"
#include <mutex>

namespace ramses_internal
{
    class RendererEventCollector
    {
    public:
        void appendAndConsumePendingEvents(RendererEventVector& rendererEvents, RendererEventVector& sceneControlEvents);
        void dispatchInternalSceneStateEvents(InternalSceneStateEvents& resultEvents);
        RendererEventVector getRendererEvents() const;
        RendererEventVector getSceneControlEvents() const;

        void addDisplayEvent(ERendererEventType eventType, DisplayHandle displayHandle, const DisplayConfig& config = {});
        void addReadPixelsEvent(ERendererEventType eventType, DisplayHandle displayHandle, OffscreenBufferHandle offscreenBufferHandle, UInt8Vector&& pixelData);
        void addInternalSceneEvent(ERendererEventType eventType, SceneId sceneId);
        void addSceneEvent(ERendererEventType eventType, SceneId sceneId, RendererSceneState state);
        void addSceneExpirationEvent(ERendererEventType eventType, SceneId sceneId);
        void addDataLinkEvent(ERendererEventType eventType, SceneId providerSceneId, SceneId consumerSceneId, DataSlotId providerdataId, DataSlotId consumerdataId);
        void addBufferEvent(ERendererEventType eventType, OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId);
        void addBufferEvent(ERendererEventType eventType, StreamBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId);
        void addBufferEvent(ERendererEventType eventType, ExternalBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId);
        void addExternalBufferEvent(ERendererEventType eventType, DisplayHandle display, ExternalBufferHandle externalBufferHandle, uint32_t glTexId);
        void addOBEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, DisplayHandle display, int dmaBufferFD, uint32_t dmaBufferStride);
        void addSceneFlushEvent(ERendererEventType eventType, SceneId sceneId, SceneVersionTag sceneVersionTag);
        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, MouseEvent mouseEvent);
        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, KeyEvent keyEvent);
        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, ResizeEvent resizeEvent);
        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, WindowMoveEvent moveEvent);
        void addStreamSourceEvent(ERendererEventType eventType, WaylandIviSurfaceId streamSourceId);
        void addPickedEvent(ERendererEventType eventType, const SceneId& sceneId, PickableObjectIds&& pickedObjectIds);
        void addFrameTimingReport(DisplayHandle display, bool isFirstDisplay, std::chrono::microseconds maxLoopTime, std::chrono::microseconds avgLooptime);

    private:
        void pushToRendererEventQueue(RendererEvent&& newEvent);
        void pushToSceneControlEventQueue(RendererEvent&& newEvent);
        void pushToInternalSceneStateEventQueue(InternalSceneStateEvent&& newEvent);

        RendererEventVector m_rendererEvents;
        RendererEventVector m_sceneControlEvents;
        InternalSceneStateEvents m_internalSceneStateEvents;
    };
}

#endif
