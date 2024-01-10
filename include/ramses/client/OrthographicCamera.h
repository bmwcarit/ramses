//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Camera.h"

namespace ramses
{
    /**
    * @brief   The OrthographicCamera is a local camera which defines an orthographic view into the scene.
    * @details A valid camera for rendering must have viewport and frustum set, see #ramses::Camera
    *          for ways to set these parameters.
    * @ingroup CoreAPI
    */
    class RAMSES_API OrthographicCamera : public Camera
    {
    protected:
        /**
        * @brief Scene is the factory for creating OrthographicCamera instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for OrthographicCamera.
        *
        * @param[in] impl Internal data for implementation specifics of OrthographicCamera (sink - instance becomes owner)
        */
        explicit OrthographicCamera(std::unique_ptr<internal::CameraNodeImpl> impl);
    };
}
