//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOCALCAMERA_H
#define RAMSES_LOCALCAMERA_H

#include "ramses-client-api/Camera.h"

namespace ramses
{
    /**
    * @brief The LocalCamera base class is part of a scene and defines a view into the scene
    * defined by the client application.
    * The "locality" means that the client has full control over the camera
    * projection and can set viewport and frustum parameters.
    * The LocalCamera can be used for remote scenes, but it will override the renderer
    * projection parameters and viewport. It is recommended to use a RemoteCamera for
    * scenes which are supposed to be distributed.
    */
    class RAMSES_API LocalCamera : public Camera
    {
    public:
        /**
        * @brief Sets camera frustum parameters of the LocalCamera.
        *
        * @param[in] leftPlane Left plane of the camera frustum.
        *                      Left opening angle if camera is perspective.
        * @param[in] rightPlane Right plane of the camera frustum.
        *                       Right opening angle if camera is perspective.
        * @param[in] bottomPlane Bottom plane of the camera frustum.
        *                        Bottom opening angle if camera is perspective.
        * @param[in] topPlane Top plane of the camera frustum.
        *                     Top opening angle if camera is perspective.
        * @param[in] nearPlane Near plane of the camera frustum.
        * @param[in] farPlane Far plane of the camera frustum.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setFrustum(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);

        /**
        * @brief Sets the viewport of the LocalCamera. The viewport size does not have to match
        * the size of the respective RenderTarget (RenderPass binds Camera and RenderTarget).
        * However, If the sizes match, the projected camera volume will match the viewport space
        * perfectly. The viewport is aligned with the upper left corner of the RenderTarget texels
        * (or framebuffer pixels, if no RenderTarget is assigned).
        *
        * @param[in] x horizontal offset of the Viewport quad (zero = leftmost texel)
        * @param[in] y vertical offset of the Viewport quad (zero = topmost texel)
        * @param[in] width horizontal size of the Viewport quad
        * @param[in] height vertical size of the Viewport quad
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /**
        * @brief Returns the horizontal offset of the Viewport quad
        *
        * @return horizontal offset of the Viewport quad
        */
        uint32_t getViewportX() const;

        /**
        * @brief Returns the vertical offset of the Viewport quad
        *
        * @return vertical offset of the Viewport quad
        */
        uint32_t getViewportY() const;

        /**
        * @brief Returns the horizontal size of the Viewport quad
        *
        * @return horizontal size of the Viewport quad
        */
        uint32_t getViewportWidth() const;

        /**
        * @brief Returns the vertical size of the Viewport quad
        *
        * @return vertical size of the Viewport quad
        */
        uint32_t getViewportHeight() const;

        /**
        * @brief Returns the left plane of the LocalCamera
        *
        * @return the left plane of the LocalCamera
        */
        float getLeftPlane() const;

        /**
        * @brief Returns the right plane of the LocalCamera
        *
        * @return the right plane of the LocalCamera
        */
        float getRightPlane() const;

        /**
        * @brief Returns the bottom plane of the LocalCamera
        *
        * @return the bottom plane of the LocalCamera
        */
        float getBottomPlane() const;

        /**
        * @brief Returns the top plane of the LocalCamera
        *
        * @return the top plane of the LocalCamera
        */
        float getTopPlane() const;

        /**
        * @brief Returns the near plane of the LocalCamera
        *
        * @return the near plane of the LocalCamera
        */
        float getNearPlane() const;

        /**
        * @brief Returns the far plane of the LocalCamera
        *
        * @return the far plane of the LocalCamera
        */
        float getFarPlane() const;

        /**
        * @brief Gets projection matrix based on camera parameters.
        *        Projection matrix can only be retrieved after all parameters
        *        were set and are valid.
        *
        * @param[out] projectionMatrix Will be filled with the projection matrix 4x4 column-major
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getProjectionMatrix(float (&projectionMatrix)[16]) const;

    protected:
        /**
        * @brief Scene is the factory for creating LocalCamera instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for LocalCamera.
        *
        * @param[in] pimpl Internal data for implementation specifics of LocalCamera (sink - instance becomes owner)
        */
        explicit LocalCamera(CameraNodeImpl& pimpl);

        /** Protected trivial destructor to avoid deleting by user*/
        virtual ~LocalCamera();
    };
}

#endif
