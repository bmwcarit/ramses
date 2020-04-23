//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierVector4f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierVector4f::SplineBezierVector4f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierVector4f::~SplineBezierVector4f()
    {
    }

    status_t SplineBezierVector4f::setKey(splineTimeStamp_t timeStamp, float x, float y, float z, float w, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierVector4f(timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API9(status, timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        return status;
    }

    status_t SplineBezierVector4f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& w, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsVector4f(keyIndex, timeStamp, x, y, z, w, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
