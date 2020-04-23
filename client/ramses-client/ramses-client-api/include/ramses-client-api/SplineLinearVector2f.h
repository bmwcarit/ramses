//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINELINEARVECTOR2F_H
#define RAMSES_SPLINELINEARVECTOR2F_H

#include "ramses-client-api/Spline.h"
#include "ramses-client-api/AnimationTypes.h"

namespace ramses
{
    /**
    * @brief The SplineLinearVector2f stores spline keys of type Vector2f that can be used for animation with linear interpolation.
    */
    class RAMSES_API SplineLinearVector2f : public Spline
    {
    public:
        /**
        * @brief Sets a spline key at given time with given value.
        *
        * @param[in] timeStamp The time stamp for the key to be set
        * @param[in] x The first value for the key data.
        * @param[in] y The second value for the key data.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setKey(splineTimeStamp_t timeStamp, float x, float y);

        /**
        * @brief Gets key value and time stamp for a given key index.
        *
        * @param[in] keyIndex Index of a key to get values from.
        * @param[out] timeStamp The time stamp of the key.
        * @param[out] x The first value of the key data.
        * @param[out] y The second value of the key data.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& x, float& y) const;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating SplineLinearVector2f instances.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of SplineLinearVector2f
        *
        * @param[in] pimpl Internal data for implementation specifics of Spline (sink - instance becomes owner)
        */
        explicit SplineLinearVector2f(SplineImpl& pimpl);

        /**
        * @brief Destructor of the SplineLinearVector2f
        */
        virtual ~SplineLinearVector2f();
    };
}

#endif
