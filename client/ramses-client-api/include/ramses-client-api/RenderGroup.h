//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERGROUP_H
#define RAMSES_RENDERGROUP_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    class MeshNode;

    /**
    * @brief The RenderGroup is a container used to collect renderables which are supposed
    * to be rendered together. Renderables added to it can be ordered within the RenderGroup
    * so that they will be rendered in given order. RenderGroup can then be added to a RenderPass
    * (again with optional order of RenderGroup within the RenderPass) so a hierarchical ordering
    * of renderables can be achieved.
    * The RenderGroup can also contain other nested RenderGroups ordered together with
    * the renderables. If a RenderGroup with nested RenderGroups is added to a RenderPass, all
    * renderables in all nested RenderGroups will be rendered within the RenderPass as well.
    * The order inside a nested RenderGroup is local, i.e. all its renderables/RenderGroups are
    * rendered before the next renderable/RenderGroup of its parent RenderGroup.
    */
    class RAMSES_API RenderGroup : public SceneObject
    {
    public:
        /**
        * @brief Add a mesh to this RenderGroup.
        *        If a mesh is already contained in this RenderGroup only its render order is updated.
        *        Mesh can be added to multiple RenderGroup instances.
        *
        * @param[in] mesh The mesh to add to this RenderGroup.
        * @param[in] orderWithinGroup Order within the RenderGroup that will be used for rendering.
        *                             Mesh with lower number will be rendered before a mesh with higher number.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addMeshNode(const MeshNode& mesh, int32_t orderWithinGroup = 0);

        /**
        * @brief Remove a mesh from this RenderGroup.
        *
        * @param[in] mesh The mesh to remove from this RenderGroup.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeMeshNode(const MeshNode& mesh);

        /**
        * @brief Checks whether a mesh was added to this RenderGroup.
        *
        * @param[in] mesh The mesh to query
        * @return \c true if the mesh is contained in this RenderGroup \c false otherwise
        */
        [[nodiscard]] bool containsMeshNode(const MeshNode& mesh) const;

        /**
        * @brief Gets a render order of given MeshNode within this RenderGroup.
        *
        * @param[in] mesh The MeshNode to query order for
        * @param[out] orderWithinGroup Order of the MeshNode within this RenderGroup
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getMeshNodeOrder(const MeshNode& mesh, int32_t& orderWithinGroup) const;

        /**
        * @brief Add a RenderGroup to this RenderGroup.
        *        If a RenderGroup is already contained in this RenderGroup only its render order is updated.
        *        RenderGroup can be added to multiple RenderGroup instances.
        *
        * @param[in] renderGroup The RenderGroup to add to this RenderGroup.
        * @param[in] orderWithinGroup Order within the RenderGroup that will be used for rendering.
        *                             RenderGroup with lower number will be rendered before a RenderGroup with higher number.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinGroup = 0);

        /**
        * @brief Remove a RenderGroup from this RenderGroup.
        *
        * @param[in] renderGroup The RenderGroup to remove from this RenderGroup.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeRenderGroup(const RenderGroup& renderGroup);

        /**
        * @brief Checks whether a RenderGroup was added to this RenderGroup.
        *
        * @param[in] renderGroup The RenderGroup to query
        * @return \c true if the RenderGroup is contained in this RenderGroup \c false otherwise
        */
        [[nodiscard]] bool containsRenderGroup(const RenderGroup& renderGroup) const;

        /**
        * @brief Gets a render order of given RenderGroup within this RenderGroup.
        *
        * @param[in] renderGroup The RenderGroup to query order for
        * @param[out] orderWithinGroup Order of the RenderGroup within this RenderGroup
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinGroup) const;

        /**
        * @brief Will remove all renderables from this RenderGroup.
        *        Renderables in nested RenderGroups will stay.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAllRenderables();

        /**
        * @brief Will remove all RenderGroups from this RenderGroup.
        *        This is done NON-recursively, i.e. the RenderGroups themselves stay untouched.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAllRenderGroups();

        /**
        * Stores internal data for implementation specifics of RenderGroup.
        */
        class RenderGroupImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating RenderGroup instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for RenderGroup.
        *
        * @param[in] pimpl Internal data for implementation specifics of RenderGroup (sink - instance becomes owner)
        */
        explicit RenderGroup(RenderGroupImpl& pimpl);

        /**
        * @brief Destructor of the RenderGroup
        */
        ~RenderGroup() override;
    };
}

#endif
