//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CARMODELVIEWMETADATA_H
#define RAMSES_CARMODELVIEWMETADATA_H

#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    /**
    * @brief Struct containing metadata about car model views
    */
    struct CarModelViewMetadata
    {
        /**
        * @brief Equals operator
        *
        * @param[in] rhs The instance to compare to
        * @return True if equal, false otherwise
        */
        constexpr bool operator==(const CarModelViewMetadata& rhs) const
        {
            return pitch == rhs.pitch &&
                yaw == rhs.yaw &&
                distance == rhs.distance &&
                origin_x == rhs.origin_x &&
                origin_y == rhs.origin_y &&
                origin_z == rhs.origin_z &&
                cameraFOV == rhs.cameraFOV &&
                nearPlane == rhs.nearPlane &&
                farPlane == rhs.farPlane;
        }

        /**
         * @brief The inequality comparison operator
         * @param rhs The instance to compare to
         * @return True if not same, false otherwise
         */
        constexpr bool operator!=(const CarModelViewMetadata& rhs) const
        {
            return !(*this == rhs);
        }

        /// The pitch of the view
        float pitch;
        /// The yaw of the view
        float yaw;
        /// The distance of the view
        float distance;
        /// The x value of the origin
        float origin_x;
        /// The y value of the origin
        float origin_y;
        /// The z value of the origin
        float origin_z;
        /// The vertical FOV of the camera
        float cameraFOV;
        /// Camera frustum near plane
        float nearPlane;
        /// Camera frustum far plane
        float farPlane;
    };
}

#endif
