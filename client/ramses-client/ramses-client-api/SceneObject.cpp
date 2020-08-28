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
    SceneObject::SceneObject(SceneObjectImpl& pimpl)
        : ClientObject(pimpl)
        , impl(pimpl)
    {
    }

    SceneObject::~SceneObject()
    {
    }

    sceneObjectId_t SceneObject::getSceneObjectId() const
    {
        return impl.getSceneObjectId();
    }
}
