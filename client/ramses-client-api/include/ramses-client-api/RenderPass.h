//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERPASS_H
#define RAMSES_RENDERPASS_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    class Camera;
    class RenderGroup;
    class RenderTarget;

    /**
     * @brief The RenderPass is a container used to collect meshes which are supposed
     * to be rendered together.
     * @details A RenderPass has a Camera which is used for all MeshNodes
     * rendered in the RenderPass. A MeshNode is rendered only if added to a RenderPass.
     * The RenderPass can optionally have a RenderTarget used to redirect the output
     * of the RenderPass. RenderPass objects are ordered together using a render order,
     * which is also shared with BlitPass objects, i.e, RenderPass and BlitPass objects
     * can all be ordered relative to each other.
     */
    class RAMSES_API RenderPass : public SceneObject
    {
    public:
        /**
        * @brief Set the camera to use for rendering the objects of this renderpass.
        * @param camera The camera to use.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setCamera(const Camera& camera);

        /**
        * Get the camera of this RenderPass.
        * @return The camera or null if Camera has not been set.
        */
        [[nodiscard]] const Camera* getCamera() const;

        /**
        * @copydoc getCamera() const
        */
        Camera* getCamera();

        /**
        * @brief Add a RenderGroup to this RenderPass for rendering.
        * @details RenderGroup can be added to multiple RenderPass instances but cannot be added multiple
        *          times to the same instance.
        *
        * @param[in] renderGroup The RenderGroup to be added.
        * @param[in] orderWithinPass Order within the RenderPass that will be used for rendering.
        *                            RenderGroup with lower number will be rendered before a RenderGroup with higher number.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t addRenderGroup(const RenderGroup& renderGroup, int32_t orderWithinPass = 0);

        /**
        * @brief Remove a RenderGroup from this RenderPass.
        *
        * @param[in] renderGroup The RenderGroup to be removed.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeRenderGroup(const RenderGroup& renderGroup);

        /**
        * @brief Checks whether a RenderGroup is part of the RenderPass
        *
        * @param[in] renderGroup The RenderGroup to look for
        * @return \c true if the mesh is used in this RenderPass \c false otherwise
        */
        [[nodiscard]] bool containsRenderGroup(const RenderGroup& renderGroup) const;

        /**
        * @brief Gets a render order of given RenderGroup within this RenderPass.
        *
        * @param[in] renderGroup The RenderGroup to query order for
        * @param[out] orderWithinPass Order of the RenderGroup within this RenderPass
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getRenderGroupOrder(const RenderGroup& renderGroup, int32_t& orderWithinPass) const;

        /**
        * @brief Will make the RenderPass empty.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t removeAllRenderGroups();

        /**
        * @brief Set the render target for the render pass to render into.
        *
        * @param renderTarget The render target to render into, set to null for direct framebuffer rendering
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderTarget(RenderTarget* renderTarget);

        /**
        * @brief Get the render target of this render pass.
        *
        * @return The render target or null if render target has not been set.
        */
        [[nodiscard]] const RenderTarget* getRenderTarget() const;

        /**
        * @brief Set the render order for the render pass.
        * @details This order defines the order in which the render pass is executed relative to other render and blit passes.
        *          Render and blit passes with lower render order are executed first. Render and blit passes with same render order
        *          might be executed by the renderer in any order.
        *
        * The default render order is Zero.
        *
        * @param renderOrder Render order used for ordering the render pass
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderOrder(int32_t renderOrder);

        /**
        * @brief Get the render order of this render pass.
        *
        * @return The render order of this render pass.
        */
        [[nodiscard]] int32_t getRenderOrder() const;

        /**
        * @brief Set the clear color for the RenderPass (default: [0,0,0,1])
        * @details The clear color will be used to clear a render target assigned to this RenderPass
        *          if clear flag is enabled, see setClearFlag.
        *
        * @param color color channels of clear color (RGBA in 0-1 range)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setClearColor(const vec4f& color);

        /**
        * @brief Returns the clear color of the RenderPass
        *
        * @return color channels of clear color
        */
        [[nodiscard]] vec4f getClearColor() const;

        /**
        * @brief Set the clear flags which enable/disable the clearing of the render target assigned to this RenderPass(default: #ramses::EClearFlags_All)
        * @details The clear flags have no effect on render passes with no render target assigned, i.e. it is not allowed to control clearing
        *          of display buffer (whether it is framebuffer or offscreen buffer) on renderer side from within scene's render pass.
        *
        * @param clearFlags clear flags, which is a bitmask of the #ramses::EClearFlags enum
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setClearFlags(uint32_t clearFlags);

        /**
        * @brief Returns the clear flags of the RenderPass
        *
        * @returns clear flags, which is a bitmask of the #ramses::EClearFlags enum
        */
        [[nodiscard]] uint32_t getClearFlags() const;

        /**
        * @brief Enable/Disable render pass
        *
        * @param enable The enable flag which indicates if the render pass is rendered (Default:true)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setEnabled(bool enable);

        /**
        * @brief Get the enable state of the render pass
        *
        * @return Indicates if the render pass is enabled
        */
        [[nodiscard]] bool isEnabled() const;

        /**
        * @brief Set/unset render once flag - rendering of the render pass only once.
        * @details By default render pass is rendered every frame, the render once flag
        *          can be used to tell the renderer to render the render pass just once.
        *          This can be useful for pre-rendering of heavy content into render targets, etc.
        *
        *          This flag is orthogonal to all the other states of render pass,
        *          e.g. render pass is not rendered at all if it is disabled (see \c setEnabled)
        *          regardless of render once flag set or unset.
        *
        *          When render pass is enabled and render once set it is guaranteed
        *          to be rendered at least once (can be more if scene is unmapped/mapped
        *          on renderer side).
        *
        * @param enable The flag which indicates if the render pass is to be rendered only once (Default:false)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderOnce(bool enable);

        /**
        * @brief Get the render once state of the render pass
        *
        * @return Indicates if the render pass is to be rendered only once
        */
        [[nodiscard]] bool isRenderOnce() const;

        /**
        * @brief Will re-render a render once pass.
        * @details If content of the render once pass changes and needs to be rendered once more
        *          this method will guarantee the render pass to be rendered once more with
        *          the state of the scene flushed together with this command.
        *
        *          Note that Ramses does not track changes of render pass content and will not automatically
        *          re-render a render once pass if its content changed.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t retriggerRenderOnce();

        /**
        * Stores internal data for implementation specifics of RenderPass.
        */
        class RenderPassImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating RenderPass instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for RenderPass.
        *
        * @param[in] pimpl Internal data for implementation specifics of RenderPass (sink - instance becomes owner)
        */
        explicit RenderPass(RenderPassImpl& pimpl);

        /**
        * @brief Destructor of the RenderPass
        */
        ~RenderPass() override;
    };
}

#endif
