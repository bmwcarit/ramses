//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-client-api/SplineStepBool.h"

// internal
#include "SplineImpl.h"

namespace ramses
{
    SplineStepBool::SplineStepBool(SplineImpl& pimpl)
        : Spline(pimpl)
    {
    }

    SplineStepBool::~SplineStepBool()
    {
    }

    status_t SplineStepBool::setKey(splineTimeStamp_t timeStamp, bool value)
    {
        const status_t status = impl.setSplineKeyStepBool(timeStamp, value);
        LOG_HL_CLIENT_API2(status, timeStamp, value);
        return status;
    }

    status_t SplineStepBool::getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, bool& value) const
    {
        return impl.getSplineKeyBool(keyIndex, timeStamp, value);
    }
}
