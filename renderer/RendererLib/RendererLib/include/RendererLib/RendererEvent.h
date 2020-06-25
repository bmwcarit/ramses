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
#include "RendererLib/EMouseEventType.h"
#include "RendererLib/EKeyEventType.h"
#include "RendererLib/EKeyCode.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/EResourceStatus.h"
#include "Math3d/Vector2i.h"
#include "Utils/LoggingUtils.h"
#include <chrono>

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
        ERendererEventType_SceneStateChanged,

        // internal scene state event types
        // TODO vaclav remove when legacy scene control not needed
        ERendererEventType_SceneUnpublished,
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
        ERendererEventType_SceneShown,
        ERendererEventType_SceneShowFailed,
        ERendererEventType_SceneHidden,
        ERendererEventType_SceneHiddenIndirect,
        ERendererEventType_SceneHideFailed,

        ERendererEventType_SceneFlushed,
        ERendererEventType_SceneAssignedToDisplayBuffer,
        ERendererEventType_SceneAssignedToDisplayBufferFailed,
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
        ERendererEventType_WindowMoveEvent,
        ERendererEventType_WindowResizeEvent,
        ERendererEventType_StreamSurfaceAvailable,
        ERendererEventType_StreamSurfaceUnavailable,
        ERendererEventType_ObjectsPicked,
        ERendererEventType_RenderThreadPeriodicLoopTimes,
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
        "ERendererEventType_SceneStateChanged",
        "ERendererEventType_SceneUnpublished",
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
        "ERendererEventType_SceneShown",
        "ERendererEventType_SceneShowFailed",
        "ERendererEventType_SceneHidden",
        "ERendererEventType_SceneHiddenAsResultOfUnpublish",
        "ERendererEventType_SceneHideFailed",
        "ERendererEventType_SceneFlushed",
        "ERendererEventType_SceneAssignedToDisplayBuffer",
        "ERendererEventType_SceneAssignedToDisplayBufferFailed",
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
        "ERendererEventType_WindowMoveEvent",
        "ERendererEventType_WindowResizeEvent",
        "ERendererEventType_StreamSurfaceAvailable",
        "ERendererEventType_StreamSurfaceUnavailable",
        "ERendererEventType_ObjectsPicked",
        "ERendererEventType_RenderThreadPeriodicLoopTimes",
    };

    ENUM_TO_STRING(ERendererEventType, RendererEventTypeNames, ERendererEventType_NUMBER_OF_ELEMENTS);

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
        RendererEvent(ERendererEventType type = ERendererEventType_Invalid)  //NOLINT(google-explicit-constructor) for RendererEventVector creation convenience
            : eventType(type)
        {
        }

        ERendererEventType          eventType;
        SceneId                     sceneId;
        RendererSceneState          state;
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
        WindowMoveEvent             moveEvent;
        StreamTextureSourceId       streamSourceId;
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

#endif
