//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierVector4i.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierVector4i::SplineBezierVector4i(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierVector4i::~SplineBezierVector4i()
    {
    }

    status_t SplineBezierVector4i::setKey(splineTimeStamp_t timeStamp, int32_t x, int32_t y, int32_t z, int32_t w, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierVector4i(timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API9(status, timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y)
        return status;
    }

    status_t SplineBezierVector4i::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, int32_t& x, int32_t& y, int32_t& z, int32_t& w, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsVector4i(keyIndex, timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
