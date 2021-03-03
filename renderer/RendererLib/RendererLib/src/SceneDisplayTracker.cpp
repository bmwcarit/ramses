//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneDisplayTracker.h"

namespace ramses_internal
{
    void SceneDisplayTracker::setSceneOwnership(SceneId scene, DisplayHandle display)
    {
        assert(display.isValid());
        m_sceneToDisplay[scene] = display;
    }

    DisplayHandle SceneDisplayTracker::getSceneOwnership(SceneId scene) const
    {
        const auto it = m_sceneToDisplay.find(scene);
        return (it != m_sceneToDisplay.cend() ? it->second : DisplayHandle::Invalid());
    }

    bool SceneDisplayTracker::IsEventResultOfBroadcastCommand(ERendererEventType eventType)
    {
        switch (eventType)
        {
        case ERendererEventType::ScenePublished:
        case ERendererEventType::SceneUnpublished:
            return true;
        case ERendererEventType::DisplayCreated:
        case ERendererEventType::DisplayCreateFailed:
        case ERendererEventType::DisplayDestroyed:
        case ERendererEventType::DisplayDestroyFailed:
        case ERendererEventType::ReadPixelsFromFramebuffer:
        case ERendererEventType::ReadPixelsFromFramebufferFailed:
        case ERendererEventType::WarpingDataUpdated:
        case ERendererEventType::WarpingDataUpdateFailed:
        case ERendererEventType::OffscreenBufferCreated:
        case ERendererEventType::OffscreenBufferCreateFailed:
        case ERendererEventType::OffscreenBufferDestroyed:
        case ERendererEventType::OffscreenBufferDestroyFailed:
        case ERendererEventType::SceneStateChanged:
        case ERendererEventType::SceneSubscribed:
        case ERendererEventType::SceneSubscribeFailed:
        case ERendererEventType::SceneUnsubscribed:
        case ERendererEventType::SceneUnsubscribedIndirect:
        case ERendererEventType::SceneUnsubscribeFailed:
        case ERendererEventType::SceneMapped:
        case ERendererEventType::SceneMapFailed:
        case ERendererEventType::SceneUnmapped:
        case ERendererEventType::SceneUnmappedIndirect:
        case ERendererEventType::SceneUnmapFailed:
        case ERendererEventType::SceneShown:
        case ERendererEventType::SceneShowFailed:
        case ERendererEventType::SceneHidden:
        case ERendererEventType::SceneHiddenIndirect:
        case ERendererEventType::SceneHideFailed:
        case ERendererEventType::SceneFlushed:
        case ERendererEventType::SceneExpirationMonitoringEnabled:
        case ERendererEventType::SceneExpirationMonitoringDisabled:
        case ERendererEventType::SceneExpired:
        case ERendererEventType::SceneRecoveredFromExpiration:
        case ERendererEventType::SceneDataLinked:
        case ERendererEventType::SceneDataLinkFailed:
        case ERendererEventType::SceneDataBufferLinked:
        case ERendererEventType::SceneDataBufferLinkFailed:
        case ERendererEventType::SceneDataUnlinked:
        case ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange:
        case ERendererEventType::SceneDataUnlinkFailed:
        case ERendererEventType::SceneDataSlotProviderCreated:
        case ERendererEventType::SceneDataSlotProviderDestroyed:
        case ERendererEventType::SceneDataSlotConsumerCreated:
        case ERendererEventType::SceneDataSlotConsumerDestroyed:
        case ERendererEventType::WindowClosed:
        case ERendererEventType::WindowKeyEvent:
        case ERendererEventType::WindowMouseEvent:
        case ERendererEventType::WindowMoveEvent:
        case ERendererEventType::WindowResizeEvent:
        case ERendererEventType::StreamSurfaceAvailable:
        case ERendererEventType::StreamSurfaceUnavailable:
        case ERendererEventType::ObjectsPicked:
        case ERendererEventType::FrameTimingReport:
            return false;
        case ERendererEventType::Invalid:
        case ERendererEventType::NUMBER_OF_ELEMENTS:
            break;
        }

        assert(false);
        return false;
    }
}
