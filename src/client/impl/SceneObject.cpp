//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/client/SceneObject.h"

// internal
#include "SceneObjectImpl.h"

namespace ramses
{
    SceneObject::SceneObject(std::unique_ptr<internal::SceneObjectImpl> impl)
        : ClientObject{ std::move(impl) }
        , m_impl{ static_cast<internal::SceneObjectImpl&>(ClientObject::m_impl) }
    {
    }

    sceneObjectId_t SceneObject::getSceneObjectId() const
    {
        return m_impl.getSceneObjectId();
    }

    const Scene& SceneObject::getScene() const
    {
        return impl().getScene();
    }

    Scene& SceneObject::getScene()
    {
        return impl().getScene();
    }

    internal::SceneObjectImpl& SceneObject::impl()
    {
        return m_impl;
    }

    const internal::SceneObjectImpl& SceneObject::impl() const
    {
        return m_impl;
    }
}
