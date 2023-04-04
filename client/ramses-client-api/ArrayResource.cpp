//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ArrayResource.h"

#include "ArrayResourceImpl.h"

namespace ramses
{
    ArrayResource::ArrayResource(ArrayResourceImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    ArrayResource::~ArrayResource()
    {
    }

    uint32_t ArrayResource::getNumberOfElements() const
    {
        return impl.getElementCount();
    }

    EDataType ArrayResource::getDataType() const
    {
        return impl.getElementType();
    }

}
