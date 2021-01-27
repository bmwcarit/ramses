//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENT_H
#define RAMSES_RENDEREREVENT_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/RendererSceneState.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/SceneVersionTag.h"
#include "SceneAPI/WaylandIviSurfaceId.h"
#include "RendererLib/EMouseEventType.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyCode.h"
#include "RendererLib/DisplayConfig.h"
#include "Math3d/Vector2i.h"
#include "Utils/LoggingUtils.h"
#include <chrono>

namespace ramses_internal
{
    enum class ERendererEventType
    {
        Invalid = 0,
        DisplayCreated,
        DisplayCreateFailed,
        DisplayDestroyed,
        DisplayDestroyFailed,
        ReadPixelsFromFramebuffer,
        ReadPixelsFromFramebufferFailed,
        WarpingDataUpdated,
        WarpingDataUpdateFailed,
        OffscreenBufferCreated,
        OffscreenBufferCreateFailed,
        OffscreenBufferDestroyed,
        OffscreenBufferDestroyFailed,
        ScenePublished,
        SceneStateChanged,

        // internal scene state event types
        SceneUnpublished,
        SceneSubscribed,
        SceneSubscribeFailed,
        SceneUnsubscribed,
        SceneUnsubscribedIndirect,
        SceneUnsubscribeFailed,
        SceneMapped,
        SceneMapFailed,
        SceneUnmapped,
        SceneUnmappedIndirect,
        SceneUnmapFailed,
        SceneShown,
        SceneShowFailed,
        SceneHidden,
        SceneHiddenIndirect,
        SceneHideFailed,

        SceneFlushed,
        SceneExpirationMonitoringEnabled,
        SceneExpirationMonitoringDisabled,
        SceneExpired,
        SceneRecoveredFromExpiration,
        SceneDataLinked,
        SceneDataLinkFailed,
        SceneDataBufferLinked,
        SceneDataBufferLinkFailed,
        SceneDataUnlinked,
        SceneDataUnlinkedAsResultOfClientSceneChange,
        SceneDataUnlinkFailed,
        SceneDataSlotProviderCreated,
        SceneDataSlotProviderDestroyed,
        SceneDataSlotConsumerCreated,
        SceneDataSlotConsumerDestroyed,
        WindowClosed,
        WindowKeyEvent,
        WindowMouseEvent,
        WindowMoveEvent,
        WindowResizeEvent,
        StreamSurfaceAvailable,
        StreamSurfaceUnavailable,
        ObjectsPicked,
        RenderThreadPeriodicLoopTimes,
        NUMBER_OF_ELEMENTS
    };

    static const Char* RendererEventTypeNames[] =
    {
        "Invalid",
        "DisplayCreated",
        "DisplayCreateFailed",
        "DisplayDestroyed",
        "DisplayDestroyFailed",
        "ReadPixelsFromFramebuffer",
        "ReadPixelsFromFramebufferFailed",
        "WarpingDataUpdated",
        "WarpingDataUpdateFailed",
        "OffscreenBufferCreated",
        "OffscreenBufferCreateFailed",
        "OffscreenBufferDestroyed",
        "OffscreenBufferDestroyFailed",
        "ScenePublished",
        "SceneStateChanged",
        "SceneUnpublished",
        "SceneSubscribed",
        "SceneSubscribeFailed",
        "SceneUnsubscribed",
        "SceneUnsubscribedAsResultOfUnpublish",
        "SceneUnsubscribeFailed",
        "SceneMapped",
        "SceneMapFailed",
        "SceneUnmapped",
        "SceneUnmappedAsResultOfUnpublish",
        "SceneUnmapFailed",
        "SceneShown",
        "SceneShowFailed",
        "SceneHidden",
        "SceneHiddenAsResultOfUnpublish",
        "SceneHideFailed",
        "SceneFlushed",
        "SceneExpirationMonitoringEnabled",
        "SceneExpirationMonitoringDisabled",
        "SceneExpired",
        "SceneRecoveredFromExpiration",
        "SceneDataLinked",
        "SceneDataLinkFailed",
        "SceneDataBufferLinked",
        "SceneDataBufferLinkFailed",
        "SceneDataUnlinked",
        "SceneDataUnlinkedAsResultOfClientSceneChange",
        "SceneDataUnlinkFailed",
        "SceneDataSlotProviderCreated",
        "SceneDataSlotProviderDestroyed",
        "SceneDataSlotConsumerCreated",
        "SceneDataSlotConsumerDestroyed",
        "WindowClosed",
        "WindowKeyEvent",
        "WindowMouseEvent",
        "WindowMoveEvent",
        "WindowResizeEvent",
        "StreamSurfaceAvailable",
        "StreamSurfaceUnavailable",
        "ObjectsPicked",
        "RenderThreadPeriodicLoopTimes",
    };

    struct MouseEvent
    {
        EMouseEventType type = EMouseEventType_Invalid;
        Vector2i        pos;
    };

    struct WindowMoveEvent
    {
        Int32        posX;
        Int32        posY;
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

    struct RenderThreadPeriodicLoopTimes
    {
        std::chrono::microseconds maximumLoopTimeWithinPeriod;
        std::chrono::microseconds averageLoopTimeWithinPeriod;
    };

    struct RendererEvent
    {
        RendererEvent(ERendererEventType type = ERendererEventType::Invalid, SceneId sId = {})  //NOLINT(google-explicit-constructor) for RendererEventVector creation convenience
            : eventType(type)
            , sceneId(sId)
        {
        }

        ERendererEventType          eventType;
        SceneId                     sceneId;
        RendererSceneState          state = RendererSceneState::Unavailable;
        DisplayHandle               displayHandle;
        DisplayConfig               displayConfig;
        UInt8Vector                 pixelData;
        SceneId                     providerSceneId;
        SceneId                     consumerSceneId;
        DataSlotId                  providerdataId;
        DataSlotId                  consumerdataId;
        OffscreenBufferHandle       offscreenBuffer;
        StreamBufferHandle          streamBuffer;
        SceneVersionTag             sceneVersionTag;
        MouseEvent                  mouseEvent;
        ResizeEvent                 resizeEvent;
        KeyEvent                    keyEvent;
        WindowMoveEvent             moveEvent;
        WaylandIviSurfaceId         streamSourceId;
        PickableObjectIds           pickedObjectIds;
        RenderThreadPeriodicLoopTimes renderThreadLoopTimes;
    };
    using RendererEventVector = std::vector<RendererEvent>;

    struct InternalSceneStateEvent
    {
        ERendererEventType type;
        SceneId sceneId;
    };
    using InternalSceneStateEvents = std::vector<InternalSceneStateEvent>;
}

MAKE_ENUM_CLASS_PRINTABLE(ramses_internal::ERendererEventType, "ERendererEventType", ramses_internal::RendererEventTypeNames, ramses_internal::ERendererEventType::NUMBER_OF_ELEMENTS);

#endif
