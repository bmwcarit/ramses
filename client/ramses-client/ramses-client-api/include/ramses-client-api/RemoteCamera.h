//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_REMOTECAMERA_H
#define RAMSES_REMOTECAMERA_H

#include "ramses-client-api/Camera.h"

namespace ramses
{
    /**
    * @brief The RemoteCamera is part of a scene and defines a view into the scene
    * defined by the (possibly remote) renderer display where this scene is mapped.
    * The viewport is taken from the display window, and projection parameters are
    * set on the display config. A client can not set/override these values when using
    * a RemoteCamera.
    */
    class RAMSES_API RemoteCamera : public Camera
    {
    protected:
        /**
        * @brief Scene is the factory for creating RemoteCamera instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for RemoteCamera.
        *
        * @param[in] pimpl Internal data for implementation specifics of RemoteCamera (sink - instance becomes owner)
        */
        explicit RemoteCamera(CameraNodeImpl& pimpl);

        /** Protected trivial destructor to avoid deleting by user*/
        virtual ~RemoteCamera();
    };
}

#endif
