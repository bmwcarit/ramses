//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BLITPASS_H
#define RAMSES_BLITPASS_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    class RenderBuffer;

    /**
     * @brief The BlitPass blits contents of one RendeBuffer to another. The source and destination
     * RenderBuffer objects must have same type, format and dimensions. BlitPass objects are ordered together
     * using a render order, which is also shared with RenderPass objects, i.e, BlitPass and RenderPass objects
     * can all be ordered relative to each other.
     */
    class RAMSES_API BlitPass : public SceneObject
    {
    public:

        /**
        * @brief Get the source render buffer used for blitting.
        *
        * @return The source render buffer.
        */
        const RenderBuffer& getSourceRenderBuffer() const;

        /**
        * @brief Get the destination render buffer used for blitting.
        *
        * @return The destination render buffer.
        */
        const RenderBuffer& getDestinationRenderBuffer() const;

        /**
        * @brief Set the region for blitting from source and destination render buffers.
        * The source and destination regions must lie within the boundaries of source
        * and destination render buffers respectively, otherwise the function fails
        * with error status.
        * If blitting region is not set the whole buffer is blit by default.
        *
        * @param sourceX Offset on x-axis for blitting from source render buffer
        * @param sourceY Offset on u-axis for blitting from source render buffer
        * @param destinationX Offset on x-axis for blitting to destination render buffer
        * @param destinationY Offset on y-axis for blitting to destination render buffer
        * @param width Width of blitting region, used for source and destination blitting regions
        * @param height Height of blitting region, used for source and destination blitting regions
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height);


        /**
        * @brief Get the blitting region in source and destination render buffers.
        *
        * @param[out] sourceX Offset on x-axis for blitting from source render buffer
        * @param[out] sourceY Offset on u-axis for blitting from source render buffer
        * @param[out] destinationX Offset on x-axis for blitting to destination render buffer
        * @param[out] destinationY Offset on y-axis for blitting to destination render buffer
        * @param[out] width Width of blitting region, used for source and destination blitting regions
        * @param[out] height Height of blitting region, used for source and destination blitting regions
        *
        */
        void getBlittingRegion(uint32_t& sourceX, uint32_t& sourceY, uint32_t& destinationX, uint32_t& destinationY, uint32_t& width, uint32_t& height) const;

        /**
        * @brief Set the render order for the blit pass.
        * This order defines the order in which the blit pass is executed relative to other blit and render passes.
        * Blit and render passes with lower render order are executed first. Blit and render passes with same render order
        * might be executed by the renderer in any order.
        *
        * The default render order is Zero.
        *
        * @param renderOrder Render order used for ordering the blit pass
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderOrder(int32_t renderOrder);

        /**
        * @brief Get the render order of this blit pass.
        *
        * @return The render order of this blit pass.
        */
        int32_t getRenderOrder() const;

        /**
        * @brief Enable/Disable blit pass
        *
        * @param enable The enable flag which indicates if the blit pass is rendered (Default:true)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setEnabled(bool enable);

        /**
        * @brief Get the enable state of the blit pass
        *
        * @return Indicates if the blit pass is enabled
        */
        bool isEnabled() const;

        /**
        * Stores internal data for implementation specifics of BlitPass.
        */
        class BlitPassImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating BlitPass instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for BlitPass.
        *
        * @param pimpl Internal data for implementation specifics of BlitPass (sink - instance becomes owner)
        */
        explicit BlitPass(BlitPassImpl& pimpl);

        /**
        * @brief Destructor of the BlitPass
        */
        virtual ~BlitPass();
    };
}

#endif
