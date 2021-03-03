//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererScenes.h"
#include "Components/ISceneGraphConsumerComponent.h"
#include "RendererEventCollector.h"
#include "Utils/ThreadLocalLogForced.h"
#include "PlatformAbstraction/Macros.h"
#include "RendererFramework/IRendererSceneEventSender.h"

namespace ramses_internal
{
    SceneStateExecutor::SceneStateExecutor(const Renderer& renderer, IRendererSceneEventSender& rendererSceneSender, RendererEventCollector& rendererEventCollector)
        : m_renderer(renderer)
        , m_rendererEventCollector(rendererEventCollector)
        , m_rendererSceneEventSender(rendererSceneSender)
    {
    }

    void SceneStateExecutor::setPublished(SceneId sceneId, EScenePublicationMode mode)
    {
        assert(checkIfCanBePublished(sceneId));
        m_scenesStateInfo.addScene(sceneId, mode);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::ScenePublished, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene "<< sceneId << " is in state PUBLISHED");
    }

    void SceneStateExecutor::setSubscriptionRequested(SceneId sceneId)
    {
        assert(checkIfCanBeSubscriptionRequested(sceneId));
        m_rendererSceneEventSender.sendSubscribeScene(sceneId);
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::SubscriptionRequested);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state SUBSCRIPTION REQUESTED");
    }

    void SceneStateExecutor::setSubscriptionPending(SceneId sceneId)
    {
        assert(checkIfCanBeSubscriptionPending(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::SubscriptionPending);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state SUBSCRIPTION PENDING caused by SCENE RECEIVED");
    }

    void SceneStateExecutor::setSubscribed(SceneId sceneId)
    {
        assert(checkIfCanBeSubscribed(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Subscribed);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneSubscribed, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state SUBSCRIBED caused by SCENE FLUSH");
    }

    void SceneStateExecutor::rollBackToUnsubscribedAndTriggerIndirectEvents(ESceneState sceneState, SceneId sceneId)
    {
        switch (sceneState)
        {
        case ESceneState::Rendered:
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneHiddenIndirect, sceneId);
            RFALLTHROUGH;

        case ESceneState::RenderRequested:
            if (sceneState == ESceneState::RenderRequested)
            {
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShowFailed, sceneId);
            }
            RFALLTHROUGH;

        case ESceneState::Mapped:
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnmappedIndirect, sceneId);
            RFALLTHROUGH;
        case ESceneState::MappingAndUploading:
        case ESceneState::MapRequested:
            if (sceneState == ESceneState::MapRequested || sceneState == ESceneState::MappingAndUploading)
            {
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapFailed, sceneId);
            }
            RFALLTHROUGH;
        case ESceneState::Subscribed:
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnsubscribedIndirect, sceneId);
            RFALLTHROUGH;
        case ESceneState::SubscriptionPending:
        case ESceneState::SubscriptionRequested:
            if (sceneState == ESceneState::SubscriptionPending || sceneState == ESceneState::SubscriptionRequested)
            {
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneSubscribeFailed, sceneId);
            }
            // explicitly unsubscribe to avoid race in case client re-published scene while previous subscription was being processed
            m_rendererSceneEventSender.sendUnsubscribeScene(sceneId);
            break;
        default:
            break;
        }
    }

    void SceneStateExecutor::setUnpublished(SceneId sceneId)
    {
        assert(checkIfCanBeUnpublished(sceneId));
        const ESceneState sceneState = m_scenesStateInfo.getSceneState(sceneId);
        rollBackToUnsubscribedAndTriggerIndirectEvents(sceneState, sceneId);
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            m_scenesStateInfo.removeScene(sceneId);
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnpublished, sceneId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneStateExecutor::setSceneUnpublished Failed unpublish for scene with id :" << sceneId << " because state is inconsistent (state is " << EnumToString(sceneState) << ")!");
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state UNPUBLISHED, scene was in state " << EnumToString(sceneState));
    }

    void SceneStateExecutor::setUnsubscribed(SceneId sceneId, bool indirect)
    {
        if (indirect)
        {
            const ESceneState sceneState = m_scenesStateInfo.getSceneState(sceneId);
            rollBackToUnsubscribedAndTriggerIndirectEvents(sceneState, sceneId);
        }
        else
        {
            assert(canBeUnsubscribed(sceneId));
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnsubscribed, sceneId);
            m_rendererSceneEventSender.sendUnsubscribeScene(sceneId);
        }

        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Published);

        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state PUBLISHED caused by command UNSUBSCRIBE (indirect: " << indirect << ")");
    }

    void SceneStateExecutor::setMapRequested(SceneId sceneId, DisplayHandle handle)
    {
        UNUSED(handle);
        assert(canBeMapRequested(sceneId, handle));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::MapRequested);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state MAP REQUESTED");
    }

    void SceneStateExecutor::setMappingAndUploading(SceneId sceneId)
    {
        assert(canBeMappingAndUploading(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::MappingAndUploading);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state MAPPING_AND_UPLOADING");
    }

    void SceneStateExecutor::setMapped(SceneId sceneId)
    {
        assert(checkIfCanBeMapped(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Mapped);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapped, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state MAPPED caused by command MAP");
    }

    void SceneStateExecutor::setUnmapped(SceneId sceneId)
    {
        assert(canBeUnmapped(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Subscribed);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnmapped, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state SUBSCRIBED caused by command UNMAP");
    }

    void SceneStateExecutor::setRenderedRequested(SceneId sceneId)
    {
        assert(canBeRenderedRequested(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::RenderRequested);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state RENDERED_REQUESTED");
    }

    void SceneStateExecutor::setRendered(SceneId sceneId)
    {
        assert(canBeShown(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Rendered);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShown, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state RENDERED caused by command SHOW");
    }

    void SceneStateExecutor::setHidden(SceneId sceneId)
    {
        assert(canBeHidden(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState::Mapped);
        m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneHidden, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId << " is in state MAPPED caused by command HIDE");
    }

    ESceneState SceneStateExecutor::getSceneState(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            return m_scenesStateInfo.getSceneState(sceneId);
        }

        return ESceneState::Unknown;
    }

    EScenePublicationMode SceneStateExecutor::getScenePublicationMode(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            return m_scenesStateInfo.getScenePublicationMode(sceneId);
        }

        return EScenePublicationMode_Unpublished;
    }

    Bool SceneStateExecutor::checkIfCanBePublished(SceneId sceneId) const
    {
        if (!m_scenesStateInfo.hasScene(sceneId))
        {
            return true;
        }

        LOG_ERROR(CONTEXT_RENDERER, "Failed publication of scene with id :" << sceneId << " because scene with this id has already been published!");
        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeUnpublished(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            const ESceneState sceneState = m_scenesStateInfo.getSceneState(sceneId);
            if (ESceneState::Unknown != sceneState)
            {
                return true;
            }

            LOG_ERROR(CONTEXT_RENDERER, "Failed unpublish for scene with id :" << sceneId << " because state is inconsistent!");
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed unpublish for scene with id :" << sceneId << " because it was not published!");
        }

        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscriptionRequested(SceneId sceneId) const
    {
        if (!canBeSubscriptionRequested(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed subscription request for scene with id :" << sceneId << " because it was not in published state!");
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneSubscribeFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeUnsubscribed(SceneId sceneId) const
    {
        if (!canBeUnsubscribed(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to unsubscribe from scene with id :" << sceneId << " because it is not in the subscribed/subscribing state!");
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnsubscribeFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeMapRequested(SceneId sceneId, DisplayHandle handle) const
    {
        if (!canBeMapRequested(sceneId, handle))
        {
            if (getSceneState(sceneId) == ESceneState::MapRequested || getSceneState(sceneId) == ESceneState::MappingAndUploading)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to map scene with id :" << sceneId << " because renderer is already mapping the scene");
            }
            else
            {
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapFailed, sceneId);
                LOG_ERROR(CONTEXT_RENDERER, "Failed to map scene with id :" << sceneId << " because scene is not in subscribed state or display does not exist (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            }
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeMapped(SceneId sceneId) const
    {
        if (!canBeMapped(sceneId))
        {
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed map for scene with id :" << sceneId << " because scene is not in mapping_and_uploading state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeUnmapped(SceneId sceneId) const
    {
        if (!canBeUnmapped(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed unmap for scene with id :" << sceneId.getValue() << " because it is not in mapped or mapping state  (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneUnmapFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeRenderedRequested(SceneId sceneId) const
    {
        if (!canBeRenderedRequested(sceneId))
        {
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShowFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed show for scene with id :" << sceneId << " because scene is not in mapped state  (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeShown(SceneId sceneId) const
    {
        if (!canBeShown(sceneId))
        {
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShowFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed show for scene with id :" << sceneId << " because scene is not in mapped state  (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeHidden(SceneId sceneId) const
    {
        if (!canBeHidden(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed hiding scene with id :" << sceneId << " because it is not in a shown state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneHideFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscriptionPending(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            if (m_scenesStateInfo.getSceneState(sceneId) == ESceneState::SubscriptionRequested)
            {
                return true;
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to receive scene with id " << sceneId << " because scene arrived before!");
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to receive scene with id " << sceneId << " because it is not known to the renderer!");
        }

        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscribed(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            if (m_scenesStateInfo.getSceneState(sceneId) == ESceneState::SubscriptionPending)
            {
                return true;
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to subscribe scene with id " << sceneId << " because scene is in wrong state " << EnumToString(m_scenesStateInfo.getSceneState(sceneId)) << "!");
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to subscribe scene with id " << sceneId << " because it is not known to the renderer!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeSubscriptionRequested(SceneId sceneId) const
    {
        const Bool sceneIsPublished = (m_scenesStateInfo.hasScene(sceneId) && ESceneState::Published == m_scenesStateInfo.getSceneState(sceneId));
        if (sceneIsPublished)
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Subscription request for scene with id :" << sceneId << " might fail because scene is in state " << EnumToString(m_scenesStateInfo.getSceneState(sceneId)));
        return false;
    }

    Bool SceneStateExecutor::canBeUnsubscribed(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            const ESceneState currentSceneState = m_scenesStateInfo.getSceneState(sceneId);
            switch (currentSceneState)
            {
            case ESceneState::SubscriptionRequested:
            case ESceneState::SubscriptionPending:
            case ESceneState::Subscribed:
                return true;
            default:
                LOG_WARN(CONTEXT_RENDERER, "Unsubscribe scene with id :" << sceneId << " might fail because scene is in state " << EnumToString(currentSceneState));
                break;
            }
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Unsubscribe scene with id :" << sceneId << " might fail because scene was not published!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeMapRequested(SceneId sceneId, DisplayHandle handle) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && ESceneState::Subscribed == m_scenesStateInfo.getSceneState(sceneId))
        {
            if (m_renderer.hasDisplayController(handle))
            {
                return true;
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "Map scene with id :" << sceneId << " on display : " << handle.asMemoryHandle() << " might fail because the display does not exist!");
            }
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Map scene with id :" << sceneId << " might fail because scene is not in subscribed state!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeMappingAndUploading(SceneId sceneId) const
    {
        return (m_scenesStateInfo.hasScene(sceneId) && ESceneState::MapRequested == m_scenesStateInfo.getSceneState(sceneId));
    }

    Bool SceneStateExecutor::canBeMapped(SceneId sceneId) const
    {
        return (m_scenesStateInfo.hasScene(sceneId) && ESceneState::MappingAndUploading == m_scenesStateInfo.getSceneState(sceneId));
    }

    Bool SceneStateExecutor::canBeUnmapped(SceneId sceneId) const
    {
        const auto sceneState = m_scenesStateInfo.hasScene(sceneId) ? m_scenesStateInfo.getSceneState(sceneId) : ESceneState::Unknown;
        if (sceneState == ESceneState::Mapped ||
            sceneState == ESceneState::MapRequested ||
            sceneState == ESceneState::MappingAndUploading)
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Unmap scene with id :" << sceneId << " might fail because scene is not in mapped or mapping state  (state is " << EnumToString(getSceneState(sceneId)) << ")");
        return false;
    }

    Bool SceneStateExecutor::canBeRenderedRequested(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && ESceneState::Mapped == m_scenesStateInfo.getSceneState(sceneId))
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Show scene with id :" << sceneId << " might fail because scene is not in mapped state (state is " << EnumToString(getSceneState(sceneId)) << ")");
        return false;
    }

    Bool SceneStateExecutor::canBeShown(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && ESceneState::RenderRequested == m_scenesStateInfo.getSceneState(sceneId))
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Show scene with id :" << sceneId << " might fail because scene is not in rendered requested state (state is " << EnumToString(getSceneState(sceneId)) << ")");
        return false;
    }

    Bool SceneStateExecutor::canBeHidden(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId)
            && (m_scenesStateInfo.getSceneState(sceneId) == ESceneState::Rendered || m_scenesStateInfo.getSceneState(sceneId) == ESceneState::RenderRequested))
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Hide scene with id :" << sceneId << " might fail because scene is not in rendered state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
        return false;
    }

}
