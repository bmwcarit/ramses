//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/RenderPassGroupIterator.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "RenderPassImpl.h"
#include "RenderGroupImpl.h"
#include "IteratorImpl.h"
#include "RamsesObjectTypeUtils.h"

namespace ramses
{
    RenderPassGroupIterator::RenderPassGroupIterator(const RenderPass& renderPass)
        : m_impl{ std::make_unique<IteratorImpl<const RenderGroupImpl*>>(renderPass.m_impl.getAllRenderGroups()) }
    {
    }

    RenderPassGroupIterator::~RenderPassGroupIterator() = default;

    const RenderGroup* RenderPassGroupIterator::getNext()
    {
        const RenderGroupImpl* renderGroup = m_impl->getNext();
        if (renderGroup == nullptr)
            return nullptr;

        return &RamsesObjectTypeUtils::ConvertTo<RenderGroup>(renderGroup->getRamsesObject());
    }
}
