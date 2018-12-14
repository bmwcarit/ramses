//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERSPECTIVECAMERA_H
#define RAMSES_PERSPECTIVECAMERA_H

#include "ramses-client-api/LocalCamera.h"

namespace ramses
{
    /**
    * @brief The PerspectiveCamera is a local camera which defines a perspective view into
    * the scene. Frustum planes can be set explicitly via setFrustum of base class LocalCamera.
    * Alternatively a symmetric frustum can be set using field of view and aspect ratio.
    * The top and bottom planes are defined by the "vertical field of view" parameter, which specifies the angle
    * between top and bottom plane.
    */
    class RAMSES_API PerspectiveCamera : public LocalCamera
    {
    public:
        /**
        * @copydoc ramses::LocalCamera::setFrustum
        */
        using LocalCamera::setFrustum;

        /**
        * @brief An alternative API (@see LocalCamera::setFrustum) to set the projective view frustum
        *        of the camera by providing opening angle and aspect ratio.
        *
        * @param[in] fov The vertical field of view to be set.
        *                This is the full vertical opening angle.
        * @param[in] aspectRatio Ratio between frustum width and height.
        *                        This value is independent from the viewport's width and height
        * @param[in] nearPlane Near plane of the camera frustum.
        * @param[in] farPlane Far plane of the camera frustum.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setFrustum(float fov, float aspectRatio, float nearPlane, float farPlane);

        /**
        * @brief Gets the vertical field of view.
        *
        * @return Vertical field of view of this camera.
        */
        float getVerticalFieldOfView() const;

        /**
        * @brief Gets the aspect ratio between camera frustum width and heigth.
        *
        * @return Aspect ratio of this camera.
        */
        float getAspectRatio() const;

    protected:
        /**
        * @brief Scene is the factory for creating PerspectiveCamera instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for PerspectiveCamera.
        *
        * @param[in] pimpl Internal data for implementation specifics of PerspectiveCamera (sink - instance becomes owner)
        */
        explicit PerspectiveCamera(CameraNodeImpl& pimpl);

        /** Protected trivial destructor to avoid deleting by user*/
        virtual ~PerspectiveCamera();
    };
}

#endif
