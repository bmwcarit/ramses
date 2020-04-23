//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierFloat.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierFloat::SplineBezierFloat(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierFloat::~SplineBezierFloat()
    {
    }

    status_t SplineBezierFloat::setKey(splineTimeStamp_t timeStamp, float value, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierFloat(timeStamp, value, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API6(status, timeStamp, value, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        return status;
    }

    status_t SplineBezierFloat::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsFloat(keyIndex, timeStamp, value, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
