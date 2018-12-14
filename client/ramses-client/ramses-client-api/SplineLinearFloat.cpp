//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineLinearFloat.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineLinearFloat::SplineLinearFloat(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineLinearFloat::~SplineLinearFloat()
    {
    }

    status_t SplineLinearFloat::setKey(splineTimeStamp_t timeStamp, float value)
    {
        const status_t status = impl.setSplineKeyLinearFloat(timeStamp, value);
        LOG_HL_CLIENT_API2(status, timeStamp, value);
        return status;
    }

    status_t SplineLinearFloat::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value) const
    {
        return impl.getSplineKeyFloat(keyIndex, timeStamp, value);
    }
}
