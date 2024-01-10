//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/LogicNode.h"
#include <memory>

namespace ramses
{
    class Node;
    class Camera;
}

namespace ramses::internal
{
    class AnchorPointImpl;
}

namespace ramses
{
    /**
    * @brief Anchor point is a #ramses::LogicNode which calculates viewport coordinates (and depth) of a Ramses node's origin.
    * The projected coordinates are accessible via its output property and can be linked to another logic node.
    * Anchor point requires a #ramses::NodeBinding and a #ramses::CameraBinding at creation time, see #ramses::LogicEngine::createAnchorPoint.
    *
    * The update logic retrieves a model matrix from the provided Ramses node (via its binding), a projection and view matrix from the provided
    * Ramses camera (via its binding) and projects a point (0, 0, 0) (which represents origin of the given node's local space) using those matrices:
    *     projectedPoint = projectionMatrix * viewMatrix * modelMatrix * (0, 0, 0, 1)
    * Perspective correction will be applied (relevant only if using perspective camera) and then the result is transformed into viewport space of the given camera,
    * i.e. the final values are coordinates within the camera viewport.
    *
    * This means that if the given Ramses node represents a transformation of a renderable mesh, the anchor point will calculate where in viewport
    * (or on screen if viewport matches screen dimensions) the origin of the mesh would be rendered. This can be useful for example for 2D overlay
    * graphical elements or text following a 3D object.
    *
    * - Property output:
    *     - viewportCoords (#ramses::EPropertyType::Vec2f)
    *         - provides [viewportCoordX, viewportCoordY] representing the [X,Y] coordinates of the projected node transformation in viewport space
    *         - note that it is up to user if and how he decides to round the floating point values to snap to discrete pixels
    *         - note that the coordinates will not be clamped to the viewport, so viewport coordinates can be negative
    *           or larger than viewport width/height if the tracked object is outside of frustum
    *     - depth (float)
    *         - non-linear depth in [0,1] range (if within frustum), 0 at near plane, 1 at far plane of the camera frustum
    *           (equivalent to the actual value stored in depth buffer for depth testing)
    *         - note that the depth can be outside of the [0,1] range if the tracked object is outside of frustum
    *
    * Important note on update order dependency:
    * Unlike other logic nodes the calculation done inside #AnchorPoint does not depend on input properties but on states in Ramses scene instead
    * - namely node transformations and camera settings. Imagine a case where #AnchorPoint tracks a Ramses node and this node's ancestor
    * (in transformation topology) is being animated by a logic node (animation or script), i.e. the animation indirectly affects the result of #AnchorPoint.
    * As this dependency is outside of Ramses logic network (it is in Ramses transformation topology) and identifying it in runtime
    * (can change every frame) by querying Ramses is not feasible, the proper ordering of logic network update is NOT guaranteed in such case.
    * It means the #AnchorPoint calculation might be executed first before the animation is and thus giving incorrect (old) result.
    * If you end up using such setup, please use a workaround: UPDATE the #ramses::LogicEngine TWICE, right after each other in every frame/update iteration.
    *
    * Performance remark:
    * Unlike other logic nodes #AnchorPoint does not use dirtiness mechanism monitoring node's inputs which then updates
    * the outputs only if anything changed. Anchor point depends on Ramses objects and their states, which cannot be easily monitored
    * and therefore it has to be updated every time #ramses::LogicEngine::update is called. For this reason it is highly recommended
    * to keep the number of anchor nodes to a necessary minimum.
    * @ingroup LogicAPI
    */
    class RAMSES_API AnchorPoint : public LogicNode
    {
    public:
        /**
        * Returns given ramses node which is used to calculate coordinates.
        *
        * @return Ramses node to track
        */
        [[nodiscard]] const ramses::Node& getRamsesNode() const;

        /**
        * Returns given ramses camera which is used to calculate coordinates.
        *
        * @return Ramses camera associated with node to track
        */
        [[nodiscard]] const ramses::Camera& getRamsesCamera() const;

        /**
         * Get the internal data for implementation specifics of #AnchorPoint.
         */
        [[nodiscard]] internal::AnchorPointImpl& impl();

        /**
         * Get the internal data for implementation specifics of #AnchorPoint.
         */
        [[nodiscard]] const internal::AnchorPointImpl& impl() const;

    protected:
        /**
        * Constructor of AnchorPoint. User is not supposed to call this - AnchorPoints are created by other factory classes
        *
        * @param impl implementation details of the AnchorPoint
        */
        explicit AnchorPoint(std::unique_ptr<internal::AnchorPointImpl> impl) noexcept;

        /**
        * Implementation of AnchorPoint
        */
        internal::AnchorPointImpl& m_anchorPointImpl;

        friend class internal::ApiObjects;
    };
}
