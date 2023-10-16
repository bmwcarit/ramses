//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/RenderPassGroupIterator.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "impl/RenderPassImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/IteratorImpl.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses
{
    RenderPassGroupIterator::RenderPassGroupIterator(const RenderPass& renderPass)
        : m_impl{ std::make_unique<internal::IteratorImpl<const internal::RenderGroupImpl*>>(renderPass.impl().getAllRenderGroups()) }
    {
    }

    RenderPassGroupIterator::~RenderPassGroupIterator() = default;

    const RenderGroup* RenderPassGroupIterator::getNext()
    {
        const internal::RenderGroupImpl* renderGroup = m_impl->getNext();
        if (renderGroup == nullptr)
            return nullptr;

        return &internal::RamsesObjectTypeUtils::ConvertTo<RenderGroup>(renderGroup->getRamsesObject());
    }
}
