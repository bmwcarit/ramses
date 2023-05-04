//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-framework-api/APIExport.h"
#include "ramses-logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Camera;
}

namespace ramses::internal
{
    class RamsesCameraBindingImpl;
}

namespace ramses
{
    /**
     * The RamsesCameraBinding is a type of #ramses::RamsesBinding which allows the #ramses::LogicEngine to control instances of ramses::Camera.
     * RamsesCameraBinding's can be created with #ramses::LogicEngine::createRamsesCameraBinding or #ramses::LogicEngine::createRamsesCameraBindingWithFrustumPlanes,
     * which affects the set of input properties that will be used to control camera frustum as described below.
     *
     * The #RamsesCameraBinding has a static link to a ramses::Camera. After creation, #ramses::LogicNode::getInputs will
     * return a struct property with children equivalent to the camera settings of the provided ramses::Camera.
     *
     * There are two types of ramses::Camera:
     *  - ramses::PerspectiveCamera
     *  - ramses::OrthographicCamera
     *
     * Both camera types are defined through their viewport and their frustum properties. These are represented as two separate property structs
     * in the RamsesCameraBinding. Be aware if you set one or more values to one of the structs on the binding and update the LogicEngine it will lead
     * to all properties of this struct being set on the actual ramses::Camera.
     * For example if you only set the 'offsetX' of the Camera viewport it will set
     * all other viewport properties as well (offsetY, width and height) to whatever their state is at that moment.
     * The values of the #RamsesCameraBinding inputs are initialized with the values of the provided ramses::Camera during creation.
     * The frustum values of the ramses::Camera are not affected when setting viewport values, and vice-versa.
     * Check the ramses::Camera API to see which values belong together.
     * To avoid unexpected behavior, we highly recommend setting all viewport values together, and also setting all frustum planes together
     * (either by link or by setting them directly). This way unwanted behavior can be avoided.
     *
     * Since the RamsesCameraBinding derives from #ramses::RamsesBinding, it also provides the #ramses::LogicNode::getInputs
     * and #ramses::LogicNode::getOutputs method. For this class, the methods behave as follows:
     *  - #ramses::LogicNode::getInputs: returns inputs struct with two child properties: viewport and frustum.
     *          - 'viewport' (type struct) with these children:
     *              - 'offsetX' (type Int32)  - viewport offset horizontal
     *              - 'offsetY' (type Int32)  - viewport offset vertical
     *              - 'width'   (type Int32)  - viewport width
     *              - 'height'  (type Int32)  - viewport height
     *          - 'frustum' (type struct) children vary depending on frustum parameters to use, it will be one of the following sets of parameters:
     *              - full set of frustum plane properties:
     *                  - 'nearPlane'   (type Float)  - frustum plane near
     *                  - 'farPlane'    (type Float)  - frustum plane far
     *                  - 'leftPlane'   (type Float)  - frustum plane left
     *                  - 'rightPlane'  (type Float)  - frustum plane right
     *                  - 'bottomPlane' (type Float)  - frustum plane bottom
     *                  - 'topPlane'    (type Float)  - frustum plane top
     *              - simplified set of frustum properties:
     *                  - 'nearPlane'   (type Float)  - frustum plane near
     *                  - 'farPlane'    (type Float)  - frustum plane far
     *                  - 'fieldOfView' (type Float)  - frustum field of view in degrees
     *                  - 'aspectRatio' (type Float)  - aspect ratio of frustum width / frustum height
     *            Full set of frustum planes properties will be present if camera is ramses::Orthographic (regardless of which create method was used)
     *            or camera is ramses::PerspectiveCamera and #ramses::LogicEngine::createRamsesCameraBindingWithFrustumPlanes was used to create it.
     *            Simplified set of frustum properties will be present if camera is ramses::PerspectiveCamera and #ramses::LogicEngine::createRamsesCameraBinding was used to create it.
     *    Refer to ramses::Camera, ramses::PerspectiveCamera and ramses::OrthographicCamera for meaning and constraints of all these inputs.
     *
     *  - #ramses::LogicNode::getOutputs: returns always nullptr, because a #RamsesCameraBinding does not have outputs,
     *    it implicitly controls the ramses Camera
     */
    class RamsesCameraBinding : public RamsesBinding
    {
    public:
        /**
         * Returns the bound ramses camera.
         * @return the bound ramses camera
         */
        [[nodiscard]] RAMSES_API ramses::Camera& getRamsesCamera() const;

        /**
         * Implementation detail of RamsesCameraBinding
         */
        internal::RamsesCameraBindingImpl& m_cameraBinding;

    protected:
        /**
         * Constructor of RamsesCameraBinding. User is not supposed to call this - RamsesCameraBindings are created by other factory classes
         *
         * @param impl implementation details of the RamsesCameraBinding
         */
        explicit RamsesCameraBinding(std::unique_ptr<internal::RamsesCameraBindingImpl> impl) noexcept;

        friend class internal::ApiObjects;
    };
}
