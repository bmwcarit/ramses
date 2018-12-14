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
    typedef int64_t        timeMilliseconds_t;    /**< Time in milliseconds type */
    typedef uint64_t       globalTimeStamp_t;     /**< Global time stamp type */
    typedef uint32_t       splineTimeStamp_t;     /**< Spline key time stamp type */
    typedef uint32_t       sequenceTimeStamp_t;   /**< Local sequence time stamp for animation within sequence */
    typedef uint32_t       splineKeyIndex_t;      /**< Spline key index type */

    /// Time stamp within a sequence local time denoting an invalid value
    const sequenceTimeStamp_t InvalidSequenceTimeStamp = sequenceTimeStamp_t(-1);
}

#endif
