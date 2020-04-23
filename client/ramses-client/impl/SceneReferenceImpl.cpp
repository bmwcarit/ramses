//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneReferenceImpl.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Scene/ClientScene.h"
#include "SceneAPI/RendererSceneState.h"
#include "SceneObjectImpl.h"
#include "SceneImpl.h"

namespace ramses
{
    SceneReferenceImpl::SceneReferenceImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_SceneReference, name)
    {
    }

    void SceneReferenceImpl::initializeFrameworkData(sceneId_t referencedScene)
    {
        m_sceneReferenceHandle = getIScene().allocateSceneReference(ramses_internal::SceneId{ referencedScene.getValue() });
    }

    void SceneReferenceImpl::deinitializeFrameworkData()
    {
        getIScene().releaseSceneReference(m_sceneReferenceHandle);
    }

    ramses::status_t SceneReferenceImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_sceneReferenceHandle;

        return StatusOK;
    }

    ramses::status_t SceneReferenceImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_sceneReferenceHandle;

        return StatusOK;
    }

    sceneId_t SceneReferenceImpl::getReferencedSceneId() const
    {
        return sceneId_t{ getIScene().getSceneReference(m_sceneReferenceHandle).sceneId.getValue() };
    }

    status_t SceneReferenceImpl::requestState(RendererSceneState requestedState)
    {
        getIScene().requestSceneReferenceState(m_sceneReferenceHandle, GetInternalSceneReferenceState(requestedState));
        return StatusOK;
    }


    RendererSceneState SceneReferenceImpl::getRequestedState() const
    {
        return GetSceneReferenceState(getIScene().getSceneReference(m_sceneReferenceHandle).requestedState);
    }

    status_t SceneReferenceImpl::requestNotificationsForSceneVersionTags(bool flag)
    {
        getIScene().requestSceneReferenceFlushNotifications(m_sceneReferenceHandle, flag);
        return StatusOK;
    }

    status_t SceneReferenceImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setSceneReferenceRenderOrder(m_sceneReferenceHandle, renderOrder);
        return StatusOK;
    }

    ramses_internal::RendererSceneState SceneReferenceImpl::GetInternalSceneReferenceState(ramses::RendererSceneState state)
    {
        switch (state)
        {
            case RendererSceneState::Unavailable: return ramses_internal::RendererSceneState::Unavailable;
            case RendererSceneState::Available: return ramses_internal::RendererSceneState::Available;
            case RendererSceneState::Ready: return ramses_internal::RendererSceneState::Ready;
            case RendererSceneState::Rendered: return ramses_internal::RendererSceneState::Rendered;
        }
        return ramses_internal::RendererSceneState::Unavailable;
    }

    RendererSceneState SceneReferenceImpl::GetSceneReferenceState(ramses_internal::RendererSceneState state)
    {
        switch (state)
        {
        case ramses_internal::RendererSceneState::Unavailable: return RendererSceneState::Unavailable;
        case ramses_internal::RendererSceneState::Available: return RendererSceneState::Available;
        case ramses_internal::RendererSceneState::Ready: return RendererSceneState::Ready;
        case ramses_internal::RendererSceneState::Rendered: return RendererSceneState::Rendered;
        }
        return RendererSceneState::Unavailable;
    }

    ramses_internal::SceneReferenceHandle SceneReferenceImpl::getSceneReferenceHandle() const
    {
        return m_sceneReferenceHandle;
    }

    RendererSceneState SceneReferenceImpl::getReportedState() const
    {
        return m_reportedState;
    }

    void SceneReferenceImpl::setReportedState(RendererSceneState state)
    {
        m_reportedState = state;
    }

}
