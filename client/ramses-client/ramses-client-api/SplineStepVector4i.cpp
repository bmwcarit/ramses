//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineStepVector4i.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineStepVector4i::SplineStepVector4i(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineStepVector4i::~SplineStepVector4i()
    {
    }

    status_t SplineStepVector4i::setKey(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w)
    {
        const status_t status = impl.setSplineKeyStepVector4i(timeStamp, x, y, z, w);
        LOG_HL_CLIENT_API5(status, timeStamp, x, y, z, w)
        return status;
    }

    status_t SplineStepVector4i::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w) const
    {
        return impl.getSplineKeyVector4i(keyIndex, timeStamp, x, y, z, w);
    }
}
