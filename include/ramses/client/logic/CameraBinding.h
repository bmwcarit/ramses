//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/client/logic/RamsesBinding.h"

#include <memory>

namespace ramses
{
    class Camera;
}

namespace ramses::internal
{
    class CameraBindingImpl;
}

namespace ramses
{
    /**
     * #CameraBinding is a type of #ramses::RamsesBinding which allows the #ramses::LogicEngine to control instances of ramses::Camera.
     * CameraBinding can be created with #ramses::LogicEngine::createCameraBinding or #ramses::LogicEngine::createCameraBindingWithFrustumPlanes,
     * which affects the set of input properties that will be used to control camera frustum as described below.
     *
     * #CameraBinding has a static link to a ramses::Camera. After creation, #ramses::LogicNode::getInputs will
     * return a struct property with children equivalent to the camera settings of the provided ramses::Camera.
     *
     * There are two types of ramses::Camera:
     *  - ramses::PerspectiveCamera
     *  - ramses::OrthographicCamera
     *
     * Both camera types are defined through their viewport and their frustum properties. These are represented as two separate property structs
     * in the CameraBinding. Be aware if you set one or more values to one of the structs on the binding and update the LogicEngine it will lead
     * to all properties of this struct being set on the actual ramses::Camera.
     * For example if you only set the 'offsetX' of the Camera viewport it will set
     * all other viewport properties as well (offsetY, width and height) to whatever their state is at that moment.
     * The values of the #CameraBinding inputs are initialized with the values of the provided ramses::Camera during creation.
     * The frustum values of the ramses::Camera are not affected when setting viewport values, and vice-versa.
     * Check the ramses::Camera API to see which values belong together.
     * To avoid unexpected behavior, we highly recommend setting all viewport values together, and also setting all frustum planes together
     * (either by link or by setting them directly). This way unwanted behavior can be avoided.
     *
     * Since #CameraBinding derives from #ramses::RamsesBinding, it also provides the #ramses::LogicNode::getInputs
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
     *            Full set of frustum planes properties will be present if the camera is ramses::Orthographic (regardless of which create method was used)
     *            or if the camera is ramses::PerspectiveCamera and #ramses::LogicEngine::createCameraBindingWithFrustumPlanes was used to create it.
     *            Simplified set of frustum properties will be present if the camera is ramses::PerspectiveCamera and #ramses::LogicEngine::createCameraBinding was used to create it.
     *    Refer to ramses::Camera, ramses::PerspectiveCamera and ramses::OrthographicCamera for the meanings and constraints of all these inputs.
     *
     *  - #ramses::LogicNode::getOutputs: always returns nullptr, because a #CameraBinding does not have outputs,
     *    it implicitly controls the ramses Camera.
     * @ingroup LogicAPI
     */
    class RAMSES_API CameraBinding : public RamsesBinding
    {
    public:
        /**
         * Returns the bound ramses camera.
         * @return the bound ramses camera
         */
        [[nodiscard]] ramses::Camera& getRamsesCamera() const;

        /**
         * Get the internal data for implementation specifics of CameraBinding.
         */
        [[nodiscard]] internal::CameraBindingImpl& impl();

        /**
         * Get the internal data for implementation specifics of CameraBinding.
         */
        [[nodiscard]] const internal::CameraBindingImpl& impl() const;

    protected:
        /**
         * Constructor of CameraBinding. User is not supposed to call this - CameraBindings are created by other factory classes
         *
         * @param impl implementation details of the CameraBinding
         */
        explicit CameraBinding(std::unique_ptr<internal::CameraBindingImpl> impl) noexcept;

        /**
         * Implementation detail of CameraBinding
         */
        internal::CameraBindingImpl& m_cameraBinding;

        friend class internal::ApiObjects;
    };
}
