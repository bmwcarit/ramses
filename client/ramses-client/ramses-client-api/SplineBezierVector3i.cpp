//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierVector3i.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierVector3i::SplineBezierVector3i(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierVector3i::~SplineBezierVector3i()
    {
    }

    status_t SplineBezierVector3i::setKey(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierVector3i(timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API8(status, timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        return status;
    }

    status_t SplineBezierVector3i::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsVector3i(keyIndex, timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
