//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/SceneTypes.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/RendererLib/Enums/EMouseEvent.h"
#include "internal/RendererLib/Enums/EKeyEvent.h"
#include "internal/RendererLib/Enums/EKeyCode.h"
#include "internal/RendererLib/Enums/EKeyModifier.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include <chrono>

namespace ramses::internal
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
        OffscreenBufferCreated,
        OffscreenBufferCreateFailed,
        OffscreenBufferDestroyed,
        OffscreenBufferDestroyFailed,
        ExternalBufferCreated,
        ExternalBufferCreateFailed,
        ExternalBufferDestroyed,
        ExternalBufferDestroyFailed,
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
        FrameTimingReport,
    };

    const std::array RendererEventTypeNames =
    {
        "Invalid",
        "DisplayCreated",
        "DisplayCreateFailed",
        "DisplayDestroyed",
        "DisplayDestroyFailed",
        "ReadPixelsFromFramebuffer",
        "ReadPixelsFromFramebufferFailed",
        "OffscreenBufferCreated",
        "OffscreenBufferCreateFailed",
        "OffscreenBufferDestroyed",
        "OffscreenBufferDestroyFailed",
        "ExternalBufferCreated",
        "ExternalBufferCreateFailed",
        "ExternalBufferDestroyed",
        "ExternalBufferDestroyFailed",
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
        "FrameTimingReport",
    };

    struct MouseEvent
    {
        EMouseEvent type = EMouseEvent::Invalid;
        glm::ivec2  pos{};
    };

    struct WindowMoveEvent
    {
        int32_t posX = 0;
        int32_t posY = 0;
    };

    struct KeyEvent
    {
        EKeyEvent type = EKeyEvent::Invalid;
        KeyModifiers modifier;
        EKeyCode     keyCode{ramses::EKeyCode_Unknown};
    };

    struct ResizeEvent
    {
        uint32_t width{0u};
        uint32_t height{0u};
    };

    struct FrameTimings
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

        ERendererEventType          eventType = ERendererEventType::Invalid;
        SceneId                     sceneId;
        RendererSceneState          state = RendererSceneState::Unavailable;
        DisplayHandle               displayHandle;
        DisplayConfig               displayConfig;
        std::vector<uint8_t>        pixelData;
        SceneId                     providerSceneId;
        SceneId                     consumerSceneId;
        DataSlotId                  providerdataId;
        DataSlotId                  consumerdataId;
        OffscreenBufferHandle       offscreenBuffer;
        StreamBufferHandle          streamBuffer;
        ExternalBufferHandle        externalBuffer;
        SceneVersionTag             sceneVersionTag;
        MouseEvent                  mouseEvent;
        ResizeEvent                 resizeEvent;
        KeyEvent                    keyEvent;
        WindowMoveEvent             moveEvent;
        WaylandIviSurfaceId         streamSourceId;
        PickableObjectIds           pickedObjectIds;
        FrameTimings                frameTimings{};
        int                         dmaBufferFD = -1;
        uint32_t                    dmaBufferStride = 0u;
        uint32_t                    textureGlId = 0u;
    };
    using RendererEventVector = std::vector<RendererEvent>;

    struct InternalSceneStateEvent
    {
        ERendererEventType type = ERendererEventType::Invalid;
        SceneId sceneId;
    };
    using InternalSceneStateEvents = std::vector<InternalSceneStateEvent>;
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::ERendererEventType, "ERendererEventType", ramses::internal::RendererEventTypeNames, ramses::internal::ERendererEventType::FrameTimingReport);
