//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONTYPES_H
#define RAMSES_ANIMATIONTYPES_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    using timeMilliseconds_t = int64_t;    /**< Time in milliseconds type */
    using globalTimeStamp_t = uint64_t;     /**< Global time stamp type */
    using splineTimeStamp_t = uint32_t;     /**< Spline key time stamp type */
    using sequenceTimeStamp_t = uint32_t;   /**< Local sequence time stamp for animation within sequence */
    using splineKeyIndex_t = uint32_t;      /**< Spline key index type */

    /// Time stamp within a sequence local time denoting an invalid value
    const sequenceTimeStamp_t InvalidSequenceTimeStamp = sequenceTimeStamp_t(-1);
}

#endif
