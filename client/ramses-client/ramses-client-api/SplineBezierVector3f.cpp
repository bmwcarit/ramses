//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierVector3f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierVector3f::SplineBezierVector3f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierVector3f::~SplineBezierVector3f()
    {
    }

    status_t SplineBezierVector3f::setKey(splineTimeStamp_t timeStamp, float x, float y, float z, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierVector3f(timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API8(status, timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        return status;
    }

    status_t SplineBezierVector3f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& z, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsVector3f(keyIndex, timeStamp, x, y, z, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
