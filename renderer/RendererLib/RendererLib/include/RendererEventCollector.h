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
#include "Utils/LogMacros.h"
#include <mutex>

namespace ramses_internal
{
    class RendererEventCollector
    {
    public:
        void appendAndConsumePendingEvents(RendererEventVector& rendererEvents, RendererEventVector& sceneControlEvents)
        {
            rendererEvents.insert(rendererEvents.end(), m_rendererEvents.cbegin(), m_rendererEvents.cend());
            m_rendererEvents.clear();

            sceneControlEvents.insert(sceneControlEvents.end(), m_sceneControlEvents.cbegin(), m_sceneControlEvents.cend());
            m_sceneControlEvents.clear();
        }

        void dispatchInternalSceneStateEvents(InternalSceneStateEvents& resultEvents)
        {
            m_internalSceneStateEvents.swap(resultEvents);
            m_internalSceneStateEvents.clear();
        }

        RendererEventVector getRendererEvents() const
        {
            return m_rendererEvents;
        }

        RendererEventVector getSceneControlEvents() const
        {
            return m_sceneControlEvents;
        }

        void addDisplayEvent(ERendererEventType eventType, DisplayHandle displayHandle, const DisplayConfig& config = {})
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << displayHandle);

            RendererEvent event(eventType);
            event.displayHandle = displayHandle;
            event.displayConfig = config;
            pushToRendererEventQueue(std::move(event));
        }

        void addReadPixelsEvent(ERendererEventType eventType, DisplayHandle displayHandle, UInt8Vector&& pixelData)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << displayHandle);

            RendererEvent event(eventType);
            event.displayHandle = displayHandle;
            event.pixelData = std::move(pixelData);
            pushToRendererEventQueue(std::move(event));
        }

        void addInternalSceneEvent(ERendererEventType eventType, SceneId sceneId)
        {
            pushToInternalSceneStateEventQueue({ eventType, sceneId });

            // keep pushing event into general queue as well for now while supporting legacy scene control
            RendererEvent event(eventType);
            event.sceneId = sceneId;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addSceneEvent(ERendererEventType eventType, SceneId sceneId, RendererSceneState state)
        {
            RendererEvent event(eventType);
            event.sceneId = sceneId;
            event.state = state;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addSceneExpirationEvent(ERendererEventType eventType, SceneId sceneId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << sceneId.getValue());

            RendererEvent event(eventType);
            event.sceneId = sceneId;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addDataLinkEvent(ERendererEventType eventType, SceneId providerSceneId, SceneId consumerSceneId, DataSlotId providerdataId, DataSlotId consumerdataId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " providerSceneId=" << providerSceneId.getValue() << " providerDataId=" << providerdataId.getValue() << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue());

            RendererEvent event(eventType);
            event.providerSceneId = providerSceneId;
            event.consumerSceneId = consumerSceneId;
            event.providerdataId = providerdataId;
            event.consumerdataId = consumerdataId;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addOBLinkEvent(ERendererEventType eventType, OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue() << " bufferHandle=" << providerBuffer);

            RendererEvent event(eventType);
            event.offscreenBuffer = providerBuffer;
            event.consumerSceneId = consumerSceneId;
            event.consumerdataId = consumerdataId;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addOBEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, DisplayHandle display)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << display << " bufferHandle=" << buffer);

            RendererEvent event(eventType);
            event.offscreenBuffer = buffer;
            event.displayHandle = display;
            pushToRendererEventQueue(std::move(event));
        }

        void addSceneAssignEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, DisplayHandle display, SceneId scene)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << scene.getValue() << " display=" << display << " bufferHandle=" << buffer);

            RendererEvent event(eventType);
            event.displayHandle = display;
            event.offscreenBuffer = buffer;
            event.sceneId = scene;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addSceneFlushEvent(ERendererEventType eventType, SceneId sceneId, SceneVersionTag sceneVersionTag, EResourceStatus resourceStatus)
        {
            LOG_TRACE(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << sceneId.getValue() << " sceneVersionTag=" << sceneVersionTag.getValue() << " resourceStatus=" << EnumToString(resourceStatus));

            RendererEvent event(eventType);
            event.sceneId = sceneId;
            event.sceneVersionTag = sceneVersionTag;
            event.resourceStatus = resourceStatus;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, MouseEvent mouseEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.mouseEvent = mouseEvent;
            pushToRendererEventQueue(std::move(event));
        }

        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, KeyEvent keyEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.keyEvent = keyEvent;
            pushToRendererEventQueue(std::move(event));
        }

        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, ResizeEvent resizeEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.resizeEvent = resizeEvent;
            pushToRendererEventQueue(std::move(event));
        }

        void addWindowEvent(ERendererEventType eventType, DisplayHandle display, WindowMoveEvent moveEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.moveEvent = moveEvent;
            pushToRendererEventQueue(std::move(event));
        }

        void addStreamSourceEvent(ERendererEventType eventType, StreamTextureSourceId streamSourceId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " streamSourceId=" << streamSourceId.getValue());

            RendererEvent event(eventType);
            event.streamSourceId = streamSourceId;
            pushToSceneControlEventQueue(std::move(event));
        }

        void addPickedEvent(ERendererEventType eventType, const SceneId& sceneId, PickableObjectIds&& pickedObjectIds)
        {
            RendererEvent event(eventType);
            event.sceneId = sceneId;
            event.pickedObjectIds = std::move(pickedObjectIds);
            pushToSceneControlEventQueue(std::move(event));
        }

        void addRenderStatsEvent(ERendererEventType eventType, std::chrono::microseconds maximumLoopTimeInPeriod, std::chrono::microseconds renderthreadAverageLooptime)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " max loop time=" << maximumLoopTimeInPeriod.count() << "microsec, avg loop time=" << renderthreadAverageLooptime.count() << "microsec");
            RendererEvent event(eventType);
            event.renderThreadLoopTimes.maximumLoopTimeWithinPeriod = maximumLoopTimeInPeriod;
            event.renderThreadLoopTimes.averageLoopTimeWithinPeriod = renderthreadAverageLooptime;
            pushToRendererEventQueue(std::move(event));
        }

        private:
            void pushToRendererEventQueue(RendererEvent&& newEvent)
            {
                m_rendererEvents.push_back(std::move(newEvent));
            }

            void pushToSceneControlEventQueue(RendererEvent&& newEvent)
            {
                m_sceneControlEvents.push_back(std::move(newEvent));
            }

            void pushToInternalSceneStateEventQueue(InternalSceneStateEvent&& newEvent)
            {
                m_internalSceneStateEvents.push_back(std::move(newEvent));
            }

            RendererEventVector m_rendererEvents;
            RendererEventVector m_sceneControlEvents;
            InternalSceneStateEvents m_internalSceneStateEvents;
    };
}

#endif
