//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/UInt32Array.h"

// internal
#include "ArrayResourceImpl.h"

namespace ramses
{
    UInt32Array::UInt32Array(ArrayResourceImpl& pimpl)
        : Resource(pimpl)
        , impl(pimpl)
    {
    }

    UInt32Array::~UInt32Array()
    {
    }
}
