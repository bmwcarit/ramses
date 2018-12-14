//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineStepVector4f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineStepVector4f::SplineStepVector4f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineStepVector4f::~SplineStepVector4f()
    {
    }

    status_t SplineStepVector4f::setKey(splineTimeStamp_t timeStamp, float x, float y, float z, float w)
    {
        const status_t status = impl.setSplineKeyStepVector4f(timeStamp, x, y, z, w);
        LOG_HL_CLIENT_API5(status, timeStamp, x, y, z, w);
        return status;
    }

    status_t SplineStepVector4f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w) const
    {
        return impl.getSplineKeyVector4f(keyIndex, timeStamp, x, y, z, w);
    }
}
