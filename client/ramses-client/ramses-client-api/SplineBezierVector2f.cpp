//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineBezierVector2f.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineBezierVector2f::SplineBezierVector2f(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineBezierVector2f::~SplineBezierVector2f()
    {
    }

    status_t SplineBezierVector2f::setKey(splineTimeStamp_t timeStamp, float x, float y, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y)
    {
        const status_t status = impl.setSplineKeyBezierVector2f(timeStamp, x, y, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
        LOG_HL_CLIENT_API7(status, timeStamp, x, y, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y)
        return status;
    }

    status_t SplineBezierVector2f::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const
    {
        return impl.getSplineKeyTangentsVector2f(keyIndex, timeStamp, x, y, tangentIn_x, tangentIn_y, tangentOut_x, tangentOut_y);
    }
}
