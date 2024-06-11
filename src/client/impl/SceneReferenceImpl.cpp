//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneReferenceImpl.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "impl/SceneObjectImpl.h"
#include "impl/SceneImpl.h"
#include "impl/ErrorReporting.h"
#include "impl/SerializationContext.h"

namespace ramses::internal
{
    SceneReferenceImpl::SceneReferenceImpl(SceneImpl& scene, std::string_view name)
        : SceneObjectImpl(scene, ERamsesObjectType::SceneReference, name)
    {
    }

    void SceneReferenceImpl::initializeFrameworkData(sceneId_t referencedScene)
    {
        m_sceneReferenceHandle = getIScene().allocateSceneReference(ramses::internal::SceneId{referencedScene.getValue()}, {});
    }

    void SceneReferenceImpl::deinitializeFrameworkData()
    {
        getIScene().releaseSceneReference(m_sceneReferenceHandle);
    }

    bool SceneReferenceImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        outStream << m_sceneReferenceHandle;

        return true;
    }

    bool SceneReferenceImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        serializationContext.deserializeAndMap(inStream, m_sceneReferenceHandle);

        return true;
    }

    sceneId_t SceneReferenceImpl::getReferencedSceneId() const
    {
        return sceneId_t{ getIScene().getSceneReference(m_sceneReferenceHandle).sceneId.getValue() };
    }

    bool SceneReferenceImpl::requestState(RendererSceneState requestedState)
    {
        if (requestedState == RendererSceneState::Unavailable)
        {
            getErrorReporting().set("SceneReference::requestState: Can not request scene reference state Unavailable. In order to release the scene from renderer request Available state", *this);
            return false;
        }

        getIScene().requestSceneReferenceState(m_sceneReferenceHandle, requestedState);
        return true;
    }


    RendererSceneState SceneReferenceImpl::getRequestedState() const
    {
        return getIScene().getSceneReference(m_sceneReferenceHandle).requestedState;
    }

    bool SceneReferenceImpl::requestNotificationsForSceneVersionTags(bool flag)
    {
        getIScene().requestSceneReferenceFlushNotifications(m_sceneReferenceHandle, flag);
        return true;
    }

    bool SceneReferenceImpl::setRenderOrder(int32_t renderOrder)
    {
        getIScene().setSceneReferenceRenderOrder(m_sceneReferenceHandle, renderOrder);
        return true;
    }

    ramses::internal::SceneReferenceHandle SceneReferenceImpl::getSceneReferenceHandle() const
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
