//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/RenderGroupMeshIterator.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"
#include "RenderGroupImpl.h"
#include "MeshNodeImpl.h"
#include "IteratorImpl.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    RenderGroupMeshIterator::RenderGroupMeshIterator(const RenderGroup& renderGroup)
        : impl(new IteratorImpl<const MeshNodeImpl*>(renderGroup.impl.getAllMeshes()))
    {
    }

    RenderGroupMeshIterator::~RenderGroupMeshIterator()
    {
        delete impl;
    }

    const MeshNode* RenderGroupMeshIterator::getNext()
    {
        const MeshNodeImpl* meshNode = impl->getNext();
        if (meshNode == NULL)
        {
            return NULL;
        }

        return &RamsesObjectTypeUtils::ConvertTo<MeshNode>(meshNode->getRamsesObject());
    }
}
