//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/Vector4fArray.h"

// internal
#include "ArrayResourceImpl.h"

namespace ramses
{
    Vector4fArray::Vector4fArray(ArrayResourceImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    Vector4fArray::~Vector4fArray()
    {
    }
}
