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
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    SceneStateExecutor::SceneStateExecutor(const Renderer& renderer, ISceneGraphConsumerComponent& sceneGraphConsumerComponent, RendererEventCollector& rendererEventCollector)
        : m_renderer(renderer)
        , m_sceneGraphConsumerComponent(sceneGraphConsumerComponent)
        , m_rendererEventCollector(rendererEventCollector)
    {
    }

    void SceneStateExecutor::setPublished(SceneId sceneId, const Guid& clientWhereSceneIsAvailable)
    {
        assert(checkIfCanBePublished(sceneId));
        m_scenesStateInfo.addScene(sceneId, clientWhereSceneIsAvailable);
        m_rendererEventCollector.addEvent(ERendererEventType_ScenePublished, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene "<< sceneId.getValue() << " is in state PUBLISHED");
    }

    void SceneStateExecutor::setSubscriptionRequested(SceneId sceneId)
    {
        assert(checkIfCanBeSubscriptionRequested(sceneId));
        m_sceneGraphConsumerComponent.subscribeScene(m_scenesStateInfo.getSceneClientGuid(sceneId), sceneId);
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_SubscriptionRequested);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state SUBSCRIPTION REQUESTED");
    }

    void SceneStateExecutor::setSubscriptionPending(SceneId sceneId)
    {
        assert(checkIfCanBeSubscriptionPending(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_SubscriptionPending);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state SUBSCRIPTION PENDING caused by SCENE RECEIVED");
    }

    void SceneStateExecutor::setSubscribed(SceneId sceneId)
    {
        assert(checkIfCanBeSubscribed(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Subscribed);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneSubscribed, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state SUBSCRIBED caused by SCENE FLUSH");
    }

    void SceneStateExecutor::rollBackToUnsubscribedAndTriggerIndirectEvents(ESceneState sceneState, SceneId sceneId)
    {
        switch (sceneState)
        {
        case ESceneState_Rendered:
            m_rendererEventCollector.addEvent(ERendererEventType_SceneHiddenIndirect, sceneId);
        case ESceneState_Mapped:
            m_rendererEventCollector.addEvent(ERendererEventType_SceneUnmappedIndirect, sceneId);
        case ESceneState_MappingAndUploading:
        case ESceneState_MapRequested:
            if (sceneState == ESceneState_MapRequested || sceneState == ESceneState_MappingAndUploading)
            {
                m_rendererEventCollector.addEvent(ERendererEventType_SceneMapFailed, sceneId);
            }
        case ESceneState_Subscribed:
            m_rendererEventCollector.addEvent(ERendererEventType_SceneUnsubscribedIndirect, sceneId);
        case ESceneState_SubscriptionPending:
        case ESceneState_SubscriptionRequested:
            if (sceneState == ESceneState_SubscriptionPending || sceneState == ESceneState_SubscriptionRequested)
            {
                m_rendererEventCollector.addEvent(ERendererEventType_SceneSubscribeFailed, sceneId);
            }
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
            m_rendererEventCollector.addEvent(ERendererEventType_SceneUnpublished, sceneId);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneStateExecutor::setSceneUnpublished Failed unpublish for scene with id :" << sceneId.getValue() << " because state is inconsistent (state is " << sceneState << ")!");
            assert(false);
        }

        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state UNPUBLISHED, scene was in state " << EnumToString(sceneState));
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
            const ESceneState sceneState = m_scenesStateInfo.getSceneState(sceneId);
            switch (sceneState)
            {
            case ESceneState_SubscriptionRequested:
            case ESceneState_SubscriptionPending:
                m_rendererEventCollector.addEvent(ERendererEventType_SceneSubscribeFailed, sceneId);
            case ESceneState_Subscribed:
                m_rendererEventCollector.addEvent(ERendererEventType_SceneUnsubscribed, sceneId);
                break;
            default:
                assert(false);
            }
        }

        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Published);
        m_sceneGraphConsumerComponent.unsubscribeScene(m_scenesStateInfo.getSceneClientGuid(sceneId), sceneId);

        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state PUBLISHED caused by command UNSUBSCRIBE (indirect: " << indirect << ")");
    }

    void SceneStateExecutor::setMapRequested(SceneId sceneId, DisplayHandle handle)
    {
        UNUSED(handle);
        assert(canBeMapRequested(sceneId, handle));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_MapRequested);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state MAP REQUESTED");
    }

    void SceneStateExecutor::setMappingAndUploading(SceneId sceneId)
    {
        assert(canBeMappingAndUploading(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_MappingAndUploading);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state MAPPING_AND_UPLOADING");
    }

    void SceneStateExecutor::setMapped(SceneId sceneId)
    {
        assert(checkIfCanBeMapped(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Mapped);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneMapped, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state MAPPED caused by command MAP");
    }

    void SceneStateExecutor::setUnmapped(SceneId sceneId)
    {
        assert(canBeUnmapped(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Subscribed);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneUnmapped, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state SUBSCRIBED caused by command UNMAP");
    }

    void SceneStateExecutor::setRendered(SceneId sceneId)
    {
        assert(canBeShown(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Rendered);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneShown, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state RENDERED caused by command SHOW");
    }

    void SceneStateExecutor::setHidden(SceneId sceneId)
    {
        assert(canBeHidden(sceneId));
        m_scenesStateInfo.setSceneState(sceneId, ESceneState_Mapped);
        m_rendererEventCollector.addEvent(ERendererEventType_SceneHidden, sceneId);
        LOG_INFO(CONTEXT_RENDERER, "Scene " << sceneId.getValue() << " is in state MAPPED caused by command HIDE");
    }

    ESceneState SceneStateExecutor::getSceneState(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            return m_scenesStateInfo.getSceneState(sceneId);
        }

        return ESceneState_Unknown;
    }

    Bool SceneStateExecutor::checkIfCanBePublished(SceneId sceneId) const
    {
        if (!m_scenesStateInfo.hasScene(sceneId))
        {
            return true;
        }

        LOG_ERROR(CONTEXT_RENDERER, "Failed publication of scene with id :" << sceneId.getValue() << " because scene with this id has already been published!");
        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeUnpublished(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            const ESceneState sceneState = m_scenesStateInfo.getSceneState(sceneId);
            if (ESceneState_Unknown != sceneState)
            {
                return true;
            }

            LOG_ERROR(CONTEXT_RENDERER, "Failed unpublish for scene with id :" << sceneId.getValue() << " because state is inconsistent!");
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed unpublish for scene with id :" << sceneId.getValue() << " because it was not published!");
        }

        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscriptionRequested(SceneId sceneId) const
    {
        if (!canBeSubscriptionRequested(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed subscription request for scene with id :" << sceneId.getValue() << " because it was not in published state!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneSubscribeFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeUnsubscribed(SceneId sceneId) const
    {
        if (!canBeUnsubscribed(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to unsubscribe from scene with id :" << sceneId.getValue() << " because it is not in the subscribed/subscribing state!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneUnsubscribeFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeMapRequested(SceneId sceneId, DisplayHandle handle) const
    {
        if (!canBeMapRequested(sceneId, handle))
        {
            if (getSceneState(sceneId) == ESceneState_MapRequested || getSceneState(sceneId) == ESceneState_MappingAndUploading)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to map scene with id :" << sceneId.getValue() << " because renderer is already mapping the scene");
            }
            else
            {
                m_rendererEventCollector.addEvent(ERendererEventType_SceneMapFailed, sceneId);
                LOG_ERROR(CONTEXT_RENDERER, "Failed to map scene with id :" << sceneId.getValue() << " because scene is not in subscribed state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            }
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeMappingAndUploading(SceneId sceneId) const
    {
        if (!canBeMappingAndUploading(sceneId))
        {
            m_rendererEventCollector.addEvent(ERendererEventType_SceneMapFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed map for scene with id :" << sceneId.getValue() << " because scene is not in map requested state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeMapped(SceneId sceneId) const
    {
        if (!canBeMapped(sceneId))
        {
            m_rendererEventCollector.addEvent(ERendererEventType_SceneMapFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed map for scene with id :" << sceneId.getValue() << " because scene is not in mapping_and_uploading state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeUnmapped(SceneId sceneId) const
    {
        if (!canBeUnmapped(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed unmap for scene with id :" << sceneId.getValue() << " because it is not in mapped or mapping state  (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneUnmapFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeShown(SceneId sceneId) const
    {
        if (!canBeShown(sceneId))
        {
            m_rendererEventCollector.addEvent(ERendererEventType_SceneShowFailed, sceneId);
            LOG_ERROR(CONTEXT_RENDERER, "Failed show for scene with id :" << sceneId.getValue() << " because scene is not in mapped state  (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeHidden(SceneId sceneId) const
    {
        if (!canBeHidden(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed hiding scene with id :" << sceneId.getValue() << " because it is not in a shown state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
            m_rendererEventCollector.addEvent(ERendererEventType_SceneHideFailed, sceneId);
            return false;
        }

        return true;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscriptionPending(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            if (m_scenesStateInfo.getSceneState(sceneId) == ESceneState_SubscriptionRequested)
            {
                return true;
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to receive scene with id " << sceneId.getValue() << " because scene arrived before!");
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to receive scene with id " << sceneId.getValue() << " because it is not known to the renderer!");
        }

        return false;
    }

    Bool SceneStateExecutor::checkIfCanBeSubscribed(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            if (m_scenesStateInfo.getSceneState(sceneId) == ESceneState_SubscriptionPending)
            {
                return true;
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "Failed to subscribe scene with id " << sceneId.getValue() << " because scene is in wrong state " << EnumToString(m_scenesStateInfo.getSceneState(sceneId)) << "!");
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "Failed to subscribe scene with id " << sceneId.getValue() << " because it is not known to the renderer!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeSubscriptionRequested(SceneId sceneId) const
    {
        const Bool sceneIsPublished = (m_scenesStateInfo.hasScene(sceneId) && ESceneState_Published == m_scenesStateInfo.getSceneState(sceneId));
        if (sceneIsPublished)
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Subscription request for scene with id :" << sceneId.getValue() << " might fail because scene is in state " << EnumToString(m_scenesStateInfo.getSceneState(sceneId)));
        return false;
    }

    Bool SceneStateExecutor::canBeUnsubscribed(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId))
        {
            const ESceneState currentSceneState = m_scenesStateInfo.getSceneState(sceneId);
            switch (currentSceneState)
            {
            case ESceneState_SubscriptionRequested:
            case ESceneState_SubscriptionPending:
            case ESceneState_Subscribed:
                return true;
            default:
                LOG_WARN(CONTEXT_RENDERER, "Unsubscribe scene with id :" << sceneId.getValue() << " might fail because scene is in state " << EnumToString(currentSceneState));
                break;
            }
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Unsubscribe scene with id :" << sceneId.getValue() << " might fail because scene was not published!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeMapRequested(SceneId sceneId, DisplayHandle handle) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && ESceneState_Subscribed == m_scenesStateInfo.getSceneState(sceneId))
        {
            if (m_renderer.hasDisplayController(handle))
            {
                return true;
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "Map scene with id :" << sceneId.getValue() << " on display : " << handle.asMemoryHandle() << " might fail because the display does not exist!");
            }
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER, "Map scene with id :" << sceneId.getValue() << " might fail because scene is not in subscribed state!");
        }

        return false;
    }

    Bool SceneStateExecutor::canBeMappingAndUploading(SceneId sceneId) const
    {
        return (m_scenesStateInfo.hasScene(sceneId) && ESceneState_MapRequested == m_scenesStateInfo.getSceneState(sceneId));
    }

    Bool SceneStateExecutor::canBeMapped(SceneId sceneId) const
    {
        return (m_scenesStateInfo.hasScene(sceneId) && ESceneState_MappingAndUploading == m_scenesStateInfo.getSceneState(sceneId));
    }

    Bool SceneStateExecutor::canBeUnmapped(SceneId sceneId) const
    {
        const auto sceneState = m_scenesStateInfo.hasScene(sceneId) ? m_scenesStateInfo.getSceneState(sceneId) : ESceneState_Unknown;
        if (sceneState == ESceneState_Mapped || sceneState == ESceneState_MapRequested || sceneState == ESceneState_MappingAndUploading)
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Unmap scene with id :" << sceneId.getValue() << " might fail because scene is not in mapped or mapping state  (state is " << EnumToString(getSceneState(sceneId)) << ")");
        return false;
    }

    Bool SceneStateExecutor::canBeShown(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && ESceneState_Mapped == m_scenesStateInfo.getSceneState(sceneId))
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Show scene with id :" << sceneId.getValue() << " might fail because scene is not in mapped state (state is " << EnumToString(getSceneState(sceneId)) << ")");
        return false;
    }

    Bool SceneStateExecutor::canBeHidden(SceneId sceneId) const
    {
        if (m_scenesStateInfo.hasScene(sceneId) && m_scenesStateInfo.getSceneState(sceneId) == ESceneState_Rendered)
        {
            return true;
        }

        LOG_WARN(CONTEXT_RENDERER, "Hide scene with id :" << sceneId.getValue() << " might fail because scene is not in rendered state (state is " << EnumToString(getSceneState(sceneId)) << ")!");
        return false;
    }

}
