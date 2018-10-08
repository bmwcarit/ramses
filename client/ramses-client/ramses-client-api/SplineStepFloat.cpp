//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineStepFloat.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineStepFloat::SplineStepFloat(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineStepFloat::~SplineStepFloat()
    {
    }

    status_t SplineStepFloat::setKey(splineTimeStamp_t timeStamp, float value)
    {
        const status_t status = impl.setSplineKeyStepFloat(timeStamp, value);
        LOG_HL_CLIENT_API2(status, timeStamp, value)
        return status;
    }

    status_t SplineStepFloat::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value) const
    {
        return impl.getSplineKeyFloat(keyIndex, timeStamp, value);
    }
}
