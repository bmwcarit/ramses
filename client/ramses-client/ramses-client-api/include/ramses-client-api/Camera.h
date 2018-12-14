//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CAMERA_H
#define RAMSES_CAMERA_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-client-api/Node.h"

namespace ramses
{
    /**
     * @brief The Camera is part of a scene and defines a view into
     * the scene.
     */
    class RAMSES_API Camera : public Node
    {
    public:
        /**
        * Stores internal data for implementation specifics of Camera.
        */
        class CameraNodeImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating Camera instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for Camera.
        *
        * @param[in] pimpl Internal data for implementation specifics of Camera (sink - instance becomes owner)
        */
        explicit Camera(CameraNodeImpl& pimpl);

        /**
        * @brief Destructor of the Camera
        */
        virtual ~Camera();
    };
}

#endif
