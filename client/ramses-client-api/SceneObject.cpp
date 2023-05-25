//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SceneObject.h"

// internal
#include "SceneObjectImpl.h"

namespace ramses
{
    SceneObject::SceneObject(std::unique_ptr<SceneObjectImpl> impl)
        : ClientObject{ std::move(impl) }
        , m_impl{ static_cast<SceneObjectImpl&>(ClientObject::m_impl) }
    {
    }

    sceneObjectId_t SceneObject::getSceneObjectId() const
    {
        return m_impl.getSceneObjectId();
    }

    sceneId_t SceneObject::getSceneId() const
    {
        return m_impl.getSceneId();
    }
}
