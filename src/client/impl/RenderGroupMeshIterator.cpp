//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/RenderGroupMeshIterator.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"
#include "impl/RenderGroupImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/IteratorImpl.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses
{
    RenderGroupMeshIterator::RenderGroupMeshIterator(const RenderGroup& renderGroup)
        : m_impl{ std::make_unique<internal::IteratorImpl<const internal::MeshNodeImpl*>>(renderGroup.impl().getAllMeshes()) }
    {
    }

    RenderGroupMeshIterator::~RenderGroupMeshIterator() = default;

    const MeshNode* RenderGroupMeshIterator::getNext()
    {
        const internal::MeshNodeImpl* meshNode = m_impl->getNext();
        if (meshNode == nullptr)
            return nullptr;

        return &internal::RamsesObjectTypeUtils::ConvertTo<MeshNode>(meshNode->getRamsesObject());
    }
}
