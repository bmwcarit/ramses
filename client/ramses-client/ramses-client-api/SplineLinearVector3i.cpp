//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineLinearVector3i.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineLinearVector3i::SplineLinearVector3i(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineLinearVector3i::~SplineLinearVector3i()
    {
    }

    status_t SplineLinearVector3i::setKey(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z)
    {
        const status_t status = impl.setSplineKeyLinearVector3i(timeStamp, x, y, z);
        LOG_HL_CLIENT_API4(status, timeStamp, x, y, z);
        return status;
    }

    status_t SplineLinearVector3i::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z) const
    {
        return impl.getSplineKeyVector3i(keyIndex, timeStamp, x, y, z);
    }
}
