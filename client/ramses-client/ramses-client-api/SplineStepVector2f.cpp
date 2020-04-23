//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineStepVector2f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineStepVector2f::SplineStepVector2f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineStepVector2f::~SplineStepVector2f()
    {
    }

    status_t SplineStepVector2f::setKey(splineTimeStamp_t timeStamp, float x, float y)
    {
        const status_t status = impl.setSplineKeyStepVector2f(timeStamp, x, y);
        LOG_HL_CLIENT_API3(status, timeStamp, x, y);
        return status;
    }

    status_t SplineStepVector2f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y) const
    {
        return impl.getSplineKeyVector2f(keyIndex, timeStamp, x, y);
    }
}
