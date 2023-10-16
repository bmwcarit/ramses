//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/Node.h"

namespace ramses
{
    namespace internal
    {
        class MeshNodeImpl;
    }

    class Appearance;
    class Geometry;
    class Effect;

    /**
     * @ingroup CoreAPI
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
         * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
         */
        bool setAppearance(Appearance& appearance);

        /**
         * @brief Sets the Geometry of the MeshNode.
         *
         * When the given Geometry has indices set, also sets start index to 0 and index count to the number of
         * indices of geometry. When the Geometry does not provide indices, the index count has to be set by
         * setIndexCount in addition.
         *
         * @param geometry The Geometry for the MeshNode.
         * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
         */
        bool setGeometry(Geometry& geometry);

        /**
        * @brief Sets the first index of indices array that will be used for rendering.
        *
        * @param[in] startIndex First index of indices to be used (default: 0).
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setStartIndex(uint32_t startIndex);

        /**
        * @brief Gets the first index of indices array that will be used for rendering.
        * @return the first index of indices array that will be used for rendering.
        */
        [[nodiscard]] uint32_t getStartIndex() const;

        /**
        * @brief Sets the offset of the first vertex to use from each vertex array
        *        that will be used for rendering the mesh.
        *
        * @param[in] startVertex First vertex of vertex arrays to be used (default: 0).
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setStartVertex(uint32_t startVertex);

        /**
        * @brief Gets the first vertex of vertex arrays that will be used for rendering.
        * @return the first index of indices array that will be used for rendering.
        */
        [[nodiscard]] uint32_t getStartVertex() const;

        /**
        * @brief Sets the number of indices that will be used for rendering.
        *
        * @param[in] indexCount Number of indices to be used.
        *                       Default value is set from geometry binding, see #setGeometry.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setIndexCount(uint32_t indexCount);

        /**
        * @brief Gets the number of indices that will be used for rendering.
        * @return the number of indices that will be used for rendering.
        */
        [[nodiscard]] uint32_t getIndexCount() const;

        /**
        * @brief Returns the appearance.
        * @return The appearance, null on failure or if none is set.
        */
        [[nodiscard]] const Appearance* getAppearance() const;

        /**
        * @brief Removes the Appearance and Geometry previously set to the MeshNode.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool removeAppearanceAndGeometry();

        /**
        * @brief Returns the appearance.
        * @return The appearance, null on failure or if none is set.
        */
        [[nodiscard]] Appearance* getAppearance();

        /**
        * @brief Returns the geometry binding.
        * @return The geometry binding, null on failure or if none is set.
        */
        [[nodiscard]] const Geometry* getGeometry() const;

        /**
        * @brief Returns the geometry binding.
        * @return The geometry binding, null on failure or if none is set.
        */
        [[nodiscard]] Geometry* getGeometry();

        /**
        * @brief Sets the number of instances that will be drawn for this
        *        mesh by the renderer.
        *
        * @param[in] instanceCount Number of instances to be drawn (default: 1)
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setInstanceCount(uint32_t instanceCount);

        /**
        * @brief Gets the number of instance that will be drawn for this mesh
        *        by the renderer.
        * @return the number of instances of this mesh drawn on rendering.
        */
        [[nodiscard]] uint32_t getInstanceCount() const;

        /**
         * Get the internal data for implementation specifics of MeshNode.
         */
        [[nodiscard]] internal::MeshNodeImpl& impl();

        /**
         * Get the internal data for implementation specifics of MeshNode.
         */
        [[nodiscard]] const internal::MeshNodeImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating MeshNode instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for MeshNode.
        *
        * @param[in] impl Internal data for implementation specifics of MeshNode (sink - instance becomes owner)
        */
        explicit MeshNode(std::unique_ptr<internal::MeshNodeImpl> impl);

        /**
        * Stores internal data for implementation specifics of MeshNode.
        */
        internal::MeshNodeImpl& m_impl;
    };
}
