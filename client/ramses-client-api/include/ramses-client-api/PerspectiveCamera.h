//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERSPECTIVECAMERA_H
#define RAMSES_PERSPECTIVECAMERA_H

#include "ramses-client-api/Camera.h"

namespace ramses
{
    /**
    * @brief   The #PerspectiveCamera is a local camera which defines a perspective view into the scene.
    * @details A valid camera for rendering must have viewport and frustum set.
    *          Frustum planes can be set using #ramses::Camera::setFrustum or #ramses::PerspectiveCamera::setFrustum,
    *          depending if input is concrete frustum planes or field of view and aspect ratio.
    */
    class RAMSES_API PerspectiveCamera : public Camera
    {
    public:
        /**
        * @copydoc ramses::Camera::setFrustum
        */
        using Camera::setFrustum;

        /**
        * @brief   An alternative method (see #ramses::Camera::setFrustum) to set the perspective view frustum
        *          of the camera by providing opening angle and aspect ratio.
        * @details When using this method the field of view and aspect ratio are internally converted to six frustum planes,
        *          therefore this is just a convenience wrapper for #ramses::Camera::setFrustum.
        *
        *          Important note: if frustum planes data is bound (see #ramses::Camera::bindFrustumPlanes)
        *          the values set here will not be effective until unbound again, bound values are always overridden by values
        *          from bound data object. Bound values can only be modified via the #ramses::DataObject bound to them.
        *          See #ramses::RamsesUtils::SetPerspectiveCameraFrustumToDataObjects providing way to conveniently
        *          set perspective frustum on data objects also with basic validity checking.
        *
        * @param[in] fov The vertical field of view to be set, must be > 0.
        *                This is the full vertical opening angle in degrees.
        * @param[in] aspectRatio Ratio between frustum width and height, must be > 0.
        *                        This value is generally independent from the viewport width and height
        *                        but typically matches the viewport aspect ratio.
        * @param[in] nearPlane Near plane of the camera frustum, must be > 0.
        * @param[in] farPlane Far plane of the camera frustum, must be > nearPlane.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setFrustum(float fov, float aspectRatio, float nearPlane, float farPlane);

        /**
        * @brief Gets the vertical field of view opening angle in degrees.
        * @details If frustum planes data is bound (#ramses::Camera::bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return Vertical field of view of this camera.
        */
        [[nodiscard]] float getVerticalFieldOfView() const;

        /**
        * @brief Gets the aspect ratio between camera frustum width and height (set via #setFrustum, not viewport).
        * @details If frustum planes data is bound (#ramses::Camera::bindFrustumPlanes) the value returned here represents
        *          the effective value used, i.e. the one from bound #ramses::DataObject, not the one set via #setFrustum.
        *
        * @return Aspect ratio of this camera.
        */
        [[nodiscard]] float getAspectRatio() const;

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
        ~PerspectiveCamera() override;
    };
}

#endif
