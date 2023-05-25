//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ORTHOGRAPHICCAMERA_H
#define RAMSES_ORTHOGRAPHICCAMERA_H

#include "ramses-client-api/Camera.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief   The OrthographicCamera is a local camera which defines an orthographic view into the scene.
    * @details A valid camera for rendering must have viewport and frustum set, see #ramses::Camera
    *          for ways to set these parameters.
    */
    class OrthographicCamera : public Camera
    {
    protected:
        /**
        * @brief Scene is the factory for creating OrthographicCamera instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for OrthographicCamera.
        *
        * @param[in] impl Internal data for implementation specifics of OrthographicCamera (sink - instance becomes owner)
        */
        explicit OrthographicCamera(std::unique_ptr<CameraNodeImpl> impl);
    };
}

#endif
