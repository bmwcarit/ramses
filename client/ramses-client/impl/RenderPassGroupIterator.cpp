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
        : impl(new IteratorImpl<const RenderGroupImpl*>(renderPass.impl.getAllRenderGroups()))
    {
    }

    RenderPassGroupIterator::~RenderPassGroupIterator()
    {
        delete impl;
    }

    const RenderGroup* RenderPassGroupIterator::getNext()
    {
        const RenderGroupImpl* renderGroup = impl->getNext();
        if (renderGroup == NULL)
        {
            return NULL;
        }

        return &RamsesObjectTypeUtils::ConvertTo<RenderGroup>(renderGroup->getRamsesObject());
    }
}
