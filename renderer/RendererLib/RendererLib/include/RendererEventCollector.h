//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENTCOLLECTOR_H
#define RAMSES_RENDEREREVENTCOLLECTOR_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/SceneVersionTag.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/EMouseEventType.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyModifier.h"
#include "RendererLib/EKeyCode.h"
#include "Math3d/Vector2i.h"
#include "Utils/LoggingUtils.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    enum ERendererEventType
    {
        ERendererEventType_Invalid = 0,
        ERendererEventType_DisplayCreated,
        ERendererEventType_DisplayCreateFailed,
        ERendererEventType_DisplayDestroyed,
        ERendererEventType_DisplayDestroyFailed,
        ERendererEventType_ReadPixelsFromFramebuffer,
        ERendererEventType_ReadPixelsFromFramebufferFailed,
        ERendererEventType_WarpingDataUpdated,
        ERendererEventType_WarpingDataUpdateFailed,
        ERendererEventType_OffscreenBufferCreated,
        ERendererEventType_OffscreenBufferCreateFailed,
        ERendererEventType_OffscreenBufferDestroyed,
        ERendererEventType_OffscreenBufferDestroyFailed,
        ERendererEventType_ScenePublished,
        ERendererEventType_SceneUnpublished,
        ERendererEventType_SceneFlushed,
        ERendererEventType_SceneSubscribed,
        ERendererEventType_SceneSubscribeFailed,
        ERendererEventType_SceneUnsubscribed,
        ERendererEventType_SceneUnsubscribedIndirect,
        ERendererEventType_SceneUnsubscribeFailed,
        ERendererEventType_SceneMapped,
        ERendererEventType_SceneMapFailed,
        ERendererEventType_SceneUnmapped,
        ERendererEventType_SceneUnmappedIndirect,
        ERendererEventType_SceneUnmapFailed,
        ERendererEventType_SceneAssignedToOffscreenBuffer,
        ERendererEventType_SceneAssignedToOffscreenBufferFailed,
        ERendererEventType_SceneAssignedToFramebuffer,
        ERendererEventType_SceneAssignedToFramebufferFailed,
        ERendererEventType_SceneShown,
        ERendererEventType_SceneShowFailed,
        ERendererEventType_SceneHidden,
        ERendererEventType_SceneHiddenIndirect,
        ERendererEventType_SceneHideFailed,
        ERendererEventType_SceneExpired,
        ERendererEventType_SceneRecoveredFromExpiration,
        ERendererEventType_SceneDataLinked,
        ERendererEventType_SceneDataLinkFailed,
        ERendererEventType_SceneDataBufferLinked,
        ERendererEventType_SceneDataBufferLinkFailed,
        ERendererEventType_SceneDataUnlinked,
        ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange,
        ERendererEventType_SceneDataUnlinkFailed,
        ERendererEventType_SceneDataSlotProviderCreated,
        ERendererEventType_SceneDataSlotProviderDestroyed,
        ERendererEventType_SceneDataSlotConsumerCreated,
        ERendererEventType_SceneDataSlotConsumerDestroyed,
        ERendererEventType_WindowClosed,
        ERendererEventType_WindowKeyEvent,
        ERendererEventType_WindowMouseEvent,
        ERendererEventType_WindowResizeEvent,
        ERendererEventType_StreamSurfaceAvailable,
        ERendererEventType_StreamSurfaceUnavailable,

        ERendererEventType_NUMBER_OF_ELEMENTS
    };

    static const Char* RendererEventTypeNames[] =
    {
        "ERendererEventType_Invalid",
        "ERendererEventType_DisplayCreated",
        "ERendererEventType_DisplayCreateFailed",
        "ERendererEventType_DisplayDestroyed",
        "ERendererEventType_DisplayDestroyFailed",
        "ERendererEventType_ReadPixelsFromFramebuffer",
        "ERendererEventType_ReadPixelsFromFramebufferFailed",
        "ERendererEventType_WarpingDataUpdated",
        "ERendererEventType_WarpingDataUpdateFailed",
        "ERendererEventType_OffscreenBufferCreated",
        "ERendererEventType_OffscreenBufferCreateFailed",
        "ERendererEventType_OffscreenBufferDestroyed",
        "ERendererEventType_OffscreenBufferDestroyFailed",
        "ERendererEventType_ScenePublished",
        "ERendererEventType_SceneUnpublished",
        "ERendererEventType_SceneFlushed",
        "ERendererEventType_SceneSubscribed",
        "ERendererEventType_SceneSubscribeFailed",
        "ERendererEventType_SceneUnsubscribed",
        "ERendererEventType_SceneUnsubscribedAsResultOfUnpublish",
        "ERendererEventType_SceneUnsubscribeFailed",
        "ERendererEventType_SceneMapped",
        "ERendererEventType_SceneMapFailed",
        "ERendererEventType_SceneUnmapped",
        "ERendererEventType_SceneUnmappedAsResultOfUnpublish",
        "ERendererEventType_SceneUnmapFailed",
        "ERendererEventType_SceneAssignedToOffscreenBuffer",
        "ERendererEventType_SceneAssignedToOffscreenBufferFailed",
        "ERendererEventType_SceneAssignedToFramebuffer",
        "ERendererEventType_SceneAssignedToFramebufferFailed",
        "ERendererEventType_SceneShown",
        "ERendererEventType_SceneShowFailed",
        "ERendererEventType_SceneHidden",
        "ERendererEventType_SceneHiddenAsResultOfUnpublish",
        "ERendererEventType_SceneHideFailed",
        "ERendererEventType_SceneExpired",
        "ERendererEventType_SceneRecoveredFromExpiration",
        "ERendererEventType_SceneDataLinked",
        "ERendererEventType_SceneDataLinkFailed",
        "ERendererEventType_SceneDataBufferLinked",
        "ERendererEventType_SceneDataBufferLinkFailed",
        "ERendererEventType_SceneDataUnlinked",
        "ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange",
        "ERendererEventType_SceneDataUnlinkFailed",
        "ERendererEventType_SceneDataSlotProviderCreated",
        "ERendererEventType_SceneDataSlotProviderDestroyed",
        "ERendererEventType_SceneDataSlotConsumerCreated",
        "ERendererEventType_SceneDataSlotConsumerDestroyed",
        "ERendererEventType_WindowClosed",
        "ERendererEventType_WindowKeyEvent",
        "ERendererEventType_WindowMouseEvent",
        "ERendererEventType_WindowResizeEvent",
        "ERendererEventType_StreamSurfaceAvailable",
        "ERendererEventType_StreamSurfaceUnavailable"
    };

    ENUM_TO_STRING(ERendererEventType, RendererEventTypeNames, ERendererEventType_NUMBER_OF_ELEMENTS);

    struct MouseEvent
    {
        EMouseEventType type = EMouseEventType_Invalid;
        Vector2i        pos;
    };

    struct KeyEvent
    {
        EKeyEventType type = EKeyEventType_Invalid;
        UInt32        modifier;
        EKeyCode      keyCode;
    };

    struct ResizeEvent
    {
        UInt32 width;
        UInt32 height;
    };

    struct RendererEvent
    {
        RendererEvent(ERendererEventType type = ERendererEventType_Invalid)
            : eventType(type)
        {
        }

        ERendererEventType          eventType;
        SceneId                     sceneId;
        DisplayHandle               displayHandle;
        DisplayConfig               displayConfig;
        UInt8Vector                 pixelData;
        SceneId                     providerSceneId;
        SceneId                     consumerSceneId;
        DataSlotId                  providerdataId;
        DataSlotId                  consumerdataId;
        OffscreenBufferHandle       offscreenBuffer;
        SceneVersionTag             sceneVersionTag;
        EResourceStatus             resourceStatus = EResourceStatus_Unknown;
        MouseEvent                  mouseEvent;
        ResizeEvent                 resizeEvent;
        KeyEvent                    keyEvent;
        StreamTextureSourceId       streamSourceId;
    };

    using RendererEventVector = std::vector<RendererEvent>;

    class RendererEventCollector
    {
    public:
        RendererEventCollector()
        {
        }

        void dispatchEvents(RendererEventVector& resultEvents)
        {
            ramses_internal::PlatformLightweightGuard guard(m_eventsLock);
            m_events.swap(resultEvents);
            m_events.clear();
        }

        void getEvents(RendererEventVector& resultEvents) const
        {
            PlatformLightweightGuard guard(m_eventsLock);
            resultEvents = m_events;
        }

        void addEvent(ERendererEventType eventType, DisplayHandle displayHandle)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << displayHandle.asMemoryHandle());

            RendererEvent event(eventType);
            event.displayHandle = displayHandle;

            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, DisplayHandle displayHandle, UInt8Vector&& pixelData)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << displayHandle.asMemoryHandle());

            RendererEvent event(eventType);
            event.displayHandle = displayHandle;
            event.pixelData = std::move(pixelData);

            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, DisplayHandle displayHandle, const DisplayConfig& config)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << displayHandle.asMemoryHandle());

            RendererEvent event(eventType);
            event.displayHandle = displayHandle;
            event.displayConfig = config;

            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, SceneId sceneId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << sceneId.getValue());

            RendererEvent event(eventType);
            event.sceneId = sceneId;

            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, SceneId providerSceneId, SceneId consumerSceneId, DataSlotId providerdataId, DataSlotId consumerdataId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " providerSceneId=" << providerSceneId.getValue() << " providerDataId=" << providerdataId.getValue() << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue());

            RendererEvent event(eventType);
            event.providerSceneId = providerSceneId;
            event.consumerSceneId = consumerSceneId;
            event.providerdataId = providerdataId;
            event.consumerdataId = consumerdataId;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerdataId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " consumerSceneId=" << consumerSceneId.getValue() << " consumerDataId=" << consumerdataId.getValue() << " bufferHandle=" << providerBuffer.asMemoryHandle());

            RendererEvent event(eventType);
            event.offscreenBuffer = providerBuffer;
            event.consumerSceneId = consumerSceneId;
            event.consumerdataId = consumerdataId;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, DisplayHandle display)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " display=" << display.asMemoryHandle() << " bufferHandle=" << buffer.asMemoryHandle());

            RendererEvent event(eventType);
            event.offscreenBuffer = buffer;
            event.displayHandle = display;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, OffscreenBufferHandle buffer, SceneId scene)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << scene.getValue() << " bufferHandle=" << buffer.asMemoryHandle());

            RendererEvent event(eventType);
            event.offscreenBuffer = buffer;
            event.sceneId = scene;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, SceneId sceneId, SceneVersionTag sceneVersionTag, EResourceStatus resourceStatus)
        {
            LOG_TRACE(CONTEXT_RENDERER, EnumToString(eventType) << " sceneId=" << sceneId.getValue() << " sceneVersionTag=" << sceneVersionTag.getValue() << " resourceStatus=" << EnumToString(resourceStatus));

            RendererEvent event(eventType);
            event.sceneId = sceneId;
            event.sceneVersionTag = sceneVersionTag;
            event.resourceStatus = resourceStatus;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, DisplayHandle display, MouseEvent mouseEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.mouseEvent = mouseEvent;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, DisplayHandle display, KeyEvent keyEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.keyEvent = keyEvent;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, DisplayHandle display, ResizeEvent resizeEvent)
        {
            RendererEvent event(eventType);
            event.displayHandle = display;
            event.resizeEvent = resizeEvent;
            pushEventToQueue(event);
        }

        void addEvent(ERendererEventType eventType, StreamTextureSourceId streamSourceId)
        {
            LOG_INFO(CONTEXT_RENDERER, EnumToString(eventType) << " streamSourceId=" << streamSourceId.getValue());

            RendererEvent event(eventType);
            event.streamSourceId = streamSourceId;
            pushEventToQueue(event);
        }

        private:
            void pushEventToQueue(const RendererEvent& newEvent)
            {
                ramses_internal::PlatformLightweightGuard guard(m_eventsLock);
                m_events.push_back(newEvent);

                // Every 10000 messages, an error is printed to avoid internal buffer overflow of event queue
                if(0 == (m_events.size() % 10000))
                {
                    LOG_ERROR(CONTEXT_RENDERER, "RendererEventCollector::pushEventToQueue internal event queue has " << m_events.size() << " events! Looks like application is not dispatching renderer events. Possible buffer overflow of the event queue!");
                }
            }

            mutable PlatformLightweightLock m_eventsLock;
            RendererEventVector m_events;
    };
}

#endif
