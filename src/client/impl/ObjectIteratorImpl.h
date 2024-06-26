//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/IteratorImpl.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include "impl/SceneObjectRegistry.h"

namespace ramses::internal
{
    class ObjectIteratorImpl : public IteratorImpl<SceneObject*>
    {
    public:
        ObjectIteratorImpl(const SceneObjectRegistry& objRegistry, ERamsesObjectType objType)
        {
            objRegistry.getObjectsOfType(this->m_objects, objType);
            this->m_objectIterator = this->m_objects.begin();
        }
    };
}
