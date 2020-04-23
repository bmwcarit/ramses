//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINE_H
#define RAMSES_SPLINE_H

#include "ramses-client-api/AnimationObject.h"

namespace ramses
{
    /**
    * @brief The Spline is a set of keys describing an animation
    */
    class RAMSES_API Spline : public AnimationObject
    {
    public:
        /**
        * Returns number of keys stored in this spline.
        *
        * @return Number of keys
        */
        uint32_t getNumberOfKeys() const;

        /**
        * Stores internal data for implementation specifics of Spline.
        */
        class SplineImpl& impl;

    protected:
        /**
        * @brief AnimationSystemData is the factory for creating Spline instances.
        */
        friend class AnimationSystemData;

        /**
        * @brief Default constructor of Spline.
        *
        * @param[in] pimpl Internal data for implementation specifics of Spline (sink - instance becomes owner)
        */
        explicit Spline(SplineImpl& pimpl);

        /**
        * @brief Destructor of Spline
        */
        virtual ~Spline();
    };
}

#endif
