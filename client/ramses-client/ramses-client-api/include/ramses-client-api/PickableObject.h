//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PICKABLEOBJECT_H
#define RAMSES_PICKABLEOBJECT_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class VertexDataBuffer;
    class LocalCamera;

    /**
    * @brief PickableObject provides a way to specify a 'pickable' area.
    * @details The purpose of the PickableObject is to enable user to add 'pickable' components to a scene.
    *          When this area is picked (see ramses::RamsesRenderer API) a message is sent to RamsesClient with list of picked objects,
    *          these can be dispatched and handled using ramses::IRendererEventHandler::objectsPicked.
    *          Geometry specifies a triangle list (see ramses::Scene::createPickableObject for geometry requirements).
    *          PickableObject is a ramses::Node and as such can be placed in scene transformation topology,
    *          transformations will then be applied accordingly to the geometry when calculating picking.
    *          Geometry is defined in 3D coordinates but does not have to be volumetric, in fact when combined with the right camera
    *          it can represent a screen space area.
    *          In order to create a valid PickableObject it is mandatory to set a Camera. This is needed for the intersection algorithm
    *          to determine whether it was picked, the camera is needed to unproject the rasterized triangles.
    *          Typically the camera will be same as the one used to render the actual renderable but in some cases
    *          it makes sense to have different camera for rendering and picking.
    *
    *          Usage example: we have a Ramses scene with a 3D car made of multiple MeshNodes within a transformation node topology,
    *          we want to make the car pickable:
    *              1) Precompute a simplified geometry representation of the car (e.g. a bounding box) so that it can be represented as a single geometry
    *                 with minimum amount of triangles (reduce overhead of intersection computation).
    *              2) Create a PickableObject with the simplified geometry.
    *              3) Set the PickableObject's parent node to be the 3D car's root node. This way the pickable geometry will always be transformed together with the car.
    *              4) Assign the camera used to render the car to our PickableObject - the pickable geometry was computed from the car's geometry
    *                 and therefore should use same camera for both rendering and picking.
    */
    class RAMSES_API PickableObject : public Node
    {
    public:
        /**
        * @brief Get the geometry buffer assigned to this PickableObject.
        *
        * @return The geometry buffer.
        *
        **/
        const VertexDataBuffer& getGeometryBuffer() const;

        /**
        *
        * @brief Get the camera currently set to the PickableObject.
        *
        * @return The PickableObject's camera, nullptr if no camera assigned.
        *
        **/
        const LocalCamera* getCamera() const;

        /**
        *
        * @brief Set the camera to be used to unproject geometry.
        * @details Camera has to be valid (see ramses::PerspectiveCamera or ramses::OrthographicCamera)
        *          in order to be assigned to a PickableObject.
        *          See ramses::PickableObject for more details.
        *
        * @param[in] camera Camera to be used to unproject geometry.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setCamera(const LocalCamera& camera);

        /**
        *
        * @brief Get the currently set PickableObject's user ID
        *
        * @return The PickableObject's user ID
        *
        **/
        pickableObjectId_t getPickableObjectId() const;

        /**
        *
        * @brief Set PickableObject's user ID
        *
        * @param[in] id User ID assigned to PickableObject, it will be used in callback ramses::IRendererEventHandler::objectsPicked
        *            when this PickableObject is picked.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setPickableObjectId(pickableObjectId_t id);

        /**
        * @brief Enable/Disable PickableObject
        * @details If a PickableObject is disabled it cannot be picked.
        *          PickableObject is enabled when created.
        *
        * @param[in] enabled The enable flag which indicates if the PickableObject is enabled
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setEnabled(bool enabled);

        /**
        * @brief Get the enabled state of the PickableObject
        *
        * @return Indicates if the PickableObject is enabled
        */
        bool isEnabled() const;

        /**
        * Stores internal data for implementation specifics of PickableObject.
        */
        class PickableObjectImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating PickableObject instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for PickableObject.
        *
        * @param pimpl Internal data for implementation specifics of PickableObject (sink - instance becomes owner)
        */
        explicit PickableObject(PickableObjectImpl& pimpl);

        /**
        * @brief Destructor of the PickableObject
        */
        virtual ~PickableObject() override;
    };
}

#endif
