//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PROJECTIONPARAMS_H
#define RAMSES_PROJECTIONPARAMS_H

#include "Matrix44f.h"
#include "SceneAPI/ECameraProjectionType.h"

namespace ramses_internal
{
    class ProjectionParams
    {
    public:
        /**
         * @brief Creates the planes for a symmetric perspective camera frustum
         * @see https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
         * @param fieldOfViewY vertical opening angle in degrees
         * @param aspectRatio ratio of display's width over height
         * @param nearPlane near plane of the frustum
         * @param farPlane far plane of the frustum
         * @return projection parameters initialized with perspective camera frustum
         */
        static ProjectionParams Perspective(Float fieldOfViewY, Float aspectRatio, Float nearPlane, Float farPlane);

        /**
        * @brief Specify the camera parameters for orthographic projection
        * @see https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFrustum.xml
        * @param projectionType type of the camera projection (perspective or orthographic)
        * @param leftPlane left plane of the frustum
        * @param rightPlane right plane of the frustum
        * @param bottomPlane bottom plane of the frustum
        * @param topPlane top plane of the frustum
        * @param nearPlane near plane of the frustum
        * @param farPlane far plane of the frustum
        * @return projection parameters initialized with orthographic camera frustum
        */
        static ProjectionParams Frustum(ECameraProjectionType projectionType, Float leftPlane, Float rightPlane, Float bottomPlane, Float topPlane, Float nearPlane, Float farPlane);

        /**
         * @brief Helper method for re-calculating the vertical symmetric opening angle from existing projection parameters
         * @param projectionParams projection parameters
         * @return opening angle in degrees
         */
        static Float GetPerspectiveFovY(const ProjectionParams& projectionParams);

        /**
         * @brief Helper method for re-calculating aspect ration from existing projection parameters
         * @param projectionParams projection parameters
         * @return ratio of display's width to height
         */
        static Float GetAspectRatio(const ProjectionParams& projectionParams);

        bool operator==(const ProjectionParams& other) const;
        bool operator!=(const ProjectionParams& other) const;

        [[nodiscard]] ECameraProjectionType getProjectionType() const;

        [[nodiscard]] bool isValid() const;

        // frustum planes
        Float leftPlane;
        Float rightPlane;
        Float bottomPlane;
        Float topPlane;
        Float nearPlane;
        Float farPlane;

    private:
        // Don't construct projection params yourself, use static creation methods
        ProjectionParams();

        ECameraProjectionType m_projectionType;
    };
}

#endif
