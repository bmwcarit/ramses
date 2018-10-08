//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneObjectImpl.h"
#include "SceneImpl.h"

namespace ramses
{
    SceneObjectImpl::SceneObjectImpl(SceneImpl& scene, ERamsesObjectType type, const char* name)
        : ClientObjectImpl(scene.getClientImpl(), type, name)
        , m_scene(scene)
    {
        m_scene.getStatisticCollection().statObjectsCreated.incCounter(1);
    }

    SceneObjectImpl::~SceneObjectImpl()
    {
        m_scene.getStatisticCollection().statObjectsDestroyed.incCounter(1);
    }

    const SceneImpl& SceneObjectImpl::getSceneImpl() const
    {
        return m_scene;
    }

    SceneImpl& SceneObjectImpl::getSceneImpl()
    {
        return m_scene;
    }

    const ramses_internal::ClientScene& SceneObjectImpl::getIScene() const
    {
        return m_scene.getIScene();
    }

    ramses_internal::ClientScene& SceneObjectImpl::getIScene()
    {
        return m_scene.getIScene();
    }

    bool SceneObjectImpl::isFromTheSameSceneAs(const SceneObjectImpl& otherObject) const
    {
        return &getIScene() == &(otherObject.getIScene());
    }
}
