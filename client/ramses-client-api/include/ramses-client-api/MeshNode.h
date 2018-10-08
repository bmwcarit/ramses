//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MESHNODE_H
#define RAMSES_MESHNODE_H

#include "ramses-client-api/Node.h"

namespace ramses
{
    class Appearance;
    class GeometryBinding;
    class UInt16Array;
    class Vector3fArray;
    class Effect;

    /**
     * @brief The MeshNode holds all information which is
     * needed to render an object to the screen.
    */
    class RAMSES_API MeshNode : public Node
    {
    public:

        /**
         * @brief Sets the Appearance of the MeshNode.
         *
         * @param appearance The Appearance for the MeshNode.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setAppearance(Appearance& appearance);

        /**
         * @brief Sets the GeometryBinding of the MeshNode.
         *
         * When the given GeometryBinding has indices set, also sets start index to 0 and index count to the number of
         * indices of geometry. When the GeometryBinding does not provide indices, the index count has to be set by
         * setIndexCount in addition.
         *
         * @param geometry The GeometryBinding for the MeshNode.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setGeometryBinding(GeometryBinding& geometry);

        /**
        * @brief Sets the first index of indices array that will be used for rendering.
        *
        * @param[in] startIndex First index of indices to be used.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setStartIndex(uint32_t startIndex);

        /**
        * @brief Gets the first index of indices array that will be used for rendering.
        * @return the first index of indices array that will be used for rendering.
        */
        uint32_t getStartIndex() const;

        /**
        * @brief Sets the number of indices that will be used for rendering.
        *
        * @param[in] indexCount Number of indices to be used.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIndexCount(uint32_t indexCount);

        /**
        * @brief Gets the number of indices that will be used for rendering.
        * @return the number of indices that will be used for rendering.
        */
        uint32_t getIndexCount() const;

        /**
        * @brief Returns the appearance.
        * @return The appearance, null on failure or if none is set.
        */
        const Appearance* getAppearance() const;

        /**
        * @brief Removes the Appearance and GeometryBinding previously set to the MeshNode.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAppearanceAndGeometry();

        /**
        * @brief Returns the appearance.
        * @return The appearance, null on failure or if none is set.
        */
        Appearance* getAppearance();

        /**
        * @brief Returns the geometry binding.
        * @return The geometry binding, null on failure or if none is set.
        */
        const GeometryBinding* getGeometryBinding() const;

        /**
        * @brief Returns the geometry binding.
        * @return The geometry binding, null on failure or if none is set.
        */
        GeometryBinding* getGeometryBinding();

        /**
        * @brief Sets the number of instances that will be drawn for this
        *        mesh by the renderer.
        *
        * @param[in] instanceCount Number of instances to be drawn (default: 1)
        *                          Cannot be 0.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setInstanceCount(uint32_t instanceCount);

        /**
        * @brief Gets the number of instance that will be drawn for this mesh
        *        by the renderer.
        * @return the number of instances of this mesh drawn on rendering.
        */
        uint32_t getInstanceCount() const;

        /**
        * Stores internal data for implementation specifics of MeshNode.
        */
        class MeshNodeImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating MeshNode instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for MeshNode.
        *
        * @param[in] pimpl Internal data for implementation specifics of MeshNode (sink - instance becomes owner)
        */
        explicit MeshNode(MeshNodeImpl& pimpl);

        /**
        * @brief Copy constructor of MeshNode
        *
        * @param[in] other Other instance of MeshNode class
        */
        MeshNode(const MeshNode& other);

        /**
        * @brief Assignment operator of MeshNode.
        *
        * @param[in] other Other instance of MeshNode class
        * @return This instance after assignment
        */
        MeshNode& operator=(const MeshNode& other);

        /**
        * @brief Destructor of the MeshNode
        */
        virtual ~MeshNode();
    };
}

#endif
