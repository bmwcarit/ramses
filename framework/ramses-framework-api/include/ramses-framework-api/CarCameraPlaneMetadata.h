//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CARCAMERAPLANEMETADATA_H
#define RAMSES_CARCAMERAPLANEMETADATA_H

#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    /**
    * @brief Struct containing metadata about car camera planes
    */
    struct CarCameraPlaneMetadata
    {
        /**
        * @brief Equals operator
        *
        * @param[in] rhs The instance to compare to
        * @return True if equal, false otherwise
        */
        constexpr bool operator==(const CarCameraPlaneMetadata& rhs) const
        {
            return nearPlane == rhs.nearPlane &&
                farPlane == rhs.farPlane;
        }

        /**
         * @brief The inequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        constexpr bool operator!=(const CarCameraPlaneMetadata& rhs) const
        {
            return !(*this == rhs);
        }

        /// Camera near plane
        float nearPlane;

        /// Camera far plane
        float farPlane;
    };
}

#endif
