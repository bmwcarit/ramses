//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ORTHOGRAPHICCAMERA_H
#define RAMSES_ORTHOGRAPHICCAMERA_H

#include "ramses-client-api/LocalCamera.h"

namespace ramses
{
    /**
    * @brief The OrthographicCamera is a local camera which defines an orthographic view into
    * the scene.
    */
    class RAMSES_API OrthographicCamera : public LocalCamera
    {
    protected:
        /**
        * @brief Scene is the factory for creating OrthographicCamera instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for OrthographicCamera.
        *
        * @param[in] pimpl Internal data for implementation specifics of OrthographicCamera (sink - instance becomes owner)
        */
        explicit OrthographicCamera(CameraNodeImpl& pimpl);

        /** Protected trivial destructor to avoid deleting by user*/
        virtual ~OrthographicCamera();
    };
}

#endif
