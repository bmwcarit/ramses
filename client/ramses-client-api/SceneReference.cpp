//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneReferenceImpl.h"
#include "ramses-client-api/SceneReference.h"

namespace ramses
{
    SceneReference::SceneReference(SceneReferenceImpl& pimpl)
        : SceneObject(pimpl)
        , impl(pimpl)
    {
    }

    SceneReference::~SceneReference() = default;

    status_t SceneReference::requestState(RendererSceneState requestedState)
    {
        const status_t status = impl.requestState(requestedState);
        LOG_HL_CLIENT_API1(status, static_cast<uint32_t>(requestedState));
        return status;
    }

    sceneId_t SceneReference::getReferencedSceneId() const
    {
        return impl.getReferencedSceneId();
    }

    status_t SceneReference::requestNotificationsForSceneVersionTags(bool flag)
    {
        const auto status = impl.requestNotificationsForSceneVersionTags(flag);
        LOG_HL_CLIENT_API1(status, flag);
        return status;
    }

    status_t SceneReference::setRenderOrder(int32_t renderOrder)
    {
        const auto status = impl.setRenderOrder(renderOrder);
        LOG_HL_CLIENT_API1(status, renderOrder);
        return status;
    }

    ramses::RendererSceneState SceneReference::getRequestedState() const
    {
        return impl.getRequestedState();
    }
}
