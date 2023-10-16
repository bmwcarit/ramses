//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneReferenceImpl.h"
#include "ramses/client/SceneReference.h"

namespace ramses
{
    SceneReference::SceneReference(std::unique_ptr<internal::SceneReferenceImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::SceneReferenceImpl&>(SceneObject::m_impl) }
    {
    }

    bool SceneReference::requestState(RendererSceneState requestedState)
    {
        const bool status = m_impl.requestState(requestedState);
        LOG_HL_CLIENT_API1(status, static_cast<uint32_t>(requestedState));
        return status;
    }

    sceneId_t SceneReference::getReferencedSceneId() const
    {
        return m_impl.getReferencedSceneId();
    }

    bool SceneReference::requestNotificationsForSceneVersionTags(bool flag)
    {
        const auto status = m_impl.requestNotificationsForSceneVersionTags(flag);
        LOG_HL_CLIENT_API1(status, flag);
        return status;
    }

    bool SceneReference::setRenderOrder(int32_t renderOrder)
    {
        const auto status = m_impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    RendererSceneState SceneReference::getRequestedState() const
    {
        return m_impl.getRequestedState();
    }

    internal::SceneReferenceImpl& SceneReference::impl()
    {
        return m_impl;
    }

    const internal::SceneReferenceImpl& SceneReference::impl() const
    {
        return m_impl;
    }
}
