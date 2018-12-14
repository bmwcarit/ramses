//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineLinearVector3f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineLinearVector3f::SplineLinearVector3f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineLinearVector3f::~SplineLinearVector3f()
    {
    }

    status_t SplineLinearVector3f::setKey(splineTimeStamp_t timeStamp, float x, float y, float z)
    {
        const status_t status = impl.setSplineKeyLinearVector3f(timeStamp, x, y, z);
        LOG_HL_CLIENT_API4(status, timeStamp, x, y, z);
        return status;
    }

    status_t SplineLinearVector3f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z) const
    {
        return impl.getSplineKeyVector3f(keyIndex, timeStamp, x, y, z);
    }
}
