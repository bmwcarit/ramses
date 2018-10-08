//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OBJECTITERATORIMPL_H
#define RAMSES_OBJECTITERATORIMPL_H

#include "IteratorImpl.h"
#include "ramses-client-api/RamsesObjectTypes.h"
#include "RamsesObjectRegistry.h"

namespace ramses
{
    class ObjectIteratorImpl : public IteratorImpl<RamsesObject*>
    {
    public:
        ObjectIteratorImpl(const RamsesObjectRegistry& objRegistry, ERamsesObjectType objType)
            : IteratorImpl<RamsesObject*>()
        {
            objRegistry.getObjectsOfType(this->m_objects, objType);
            this->m_objectIterator = this->m_objects.begin();
        }
    };
}

#endif
