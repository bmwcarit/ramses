//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEBEZIERFLOAT_H
#define RAMSES_SPLINEBEZIERFLOAT_H

#include "ramses-client-api/Spline.h"
#include "ramses-client-api/AnimationTypes.h"

namespace ramses
{
    /**
    * @brief The SplineBezierFloat stores spline keys of type float that can be used for animation with Bezier interpolation.
    */
    class RAMSES_API SplineBezierFloat : public Spline
    {
    public:
        /**
        * @brief Sets a spline key at given time with given value.
        *
        * @param[in] timeStamp The time stamp for the key to be set
        * @param[in] value The value for the key data.
        * @param[in] tangentIn_x The X component of incoming tangent vector.
        * @param[in] tangentIn_y The Y component of incoming tangent vector.
        * @param[in] tangentOut_x The X component of outgoing tangent vector.
        * @param[in] tangentOut_y The Y component of outgoing tangent vector.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setKey(splineTimeStamp_t timeStamp, float value, float tangentIn_x, float tangentIn_y, float tangentOut_x, float tangentOut_y);

        /**
        * @brief Gets key value and time stamp for a given key index.
        *
        * @param[in] keyIndex Index of a key to get values from.
        * @param[out] timeStamp The time stamp of the key.
        * @param[out] value The value of the key data.
        * @param[out] tangentIn_x The X component of incoming tangent vector.
        * @param[out] tangentIn_y The Y component of incoming tangent vector.
        * @param[out] tangentOut_x The X component of outgoing tangent vector.
        * @param[out] tangentOut_y The Y component of outgoing tangent vector.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getKeyValues(splineKeyIndex_t keyIndex, splineTimeStamp_t& timeStamp, float& value, float& tangentIn_x, float& tangentIn_y, float& tangentOut_x, float& tangentOut_y) const;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating SplineBezierFloat instances.
        */
        friend class AnimationSystemData;

        /**
        * @brief Constructor of SplineBezierFloat
        *
        * @param[in] pimpl Internal data for implementation specifics of Spline (sink - instance becomes owner)
        */
        explicit SplineBezierFloat(SplineImpl& pimpl);

        /**
        * @brief Destructor of the SplineBezierFloat
        */
        virtual ~SplineBezierFloat();
    };
}

#endif
