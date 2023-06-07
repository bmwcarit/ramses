//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBUFFER_H
#define RAMSES_RENDERBUFFER_H

#include "ramses-client-api/SceneObject.h"
#include "ramses-client-api/TextureEnums.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief RenderBuffer can be used with RenderTarget as buffer for writing or with TextureSampler as buffer for reading.
    *
    * @details A RenderBuffer can be used by one or more RenderTargets for rendering content into it.
    *          There are two basic types of RenderBuffers - color and depth, depth buffer can have additionally stencil data.
    *          A RenderBuffer can be used by one or more TextureSamplers for sampling data from it in a shader.
    */
    class RenderBuffer : public SceneObject
    {
    public:
        /**
        * @brief Returns the width of the RenderBuffer in pixels
        *
        * @returns width in pixels
        */
        [[nodiscard]] RAMSES_API uint32_t getWidth() const;

        /**
        * @brief Returns the height of the RenderBuffer in pixels
        *
        * @returns height in pixels
        */
        [[nodiscard]] RAMSES_API uint32_t getHeight() const;

        /**
        * @brief Returns the type of the RenderBuffer
        *
        * @returns RenderBuffer type (color, depth, depth/stencil)
        */
        [[nodiscard]] RAMSES_API ERenderBufferType getBufferType() const;

        /**
        * @brief Returns the data format of the RenderBuffer
        *
        * @returns RenderBuffer data format
        */
        [[nodiscard]] RAMSES_API ERenderBufferFormat getBufferFormat() const;

        /**
        * @brief Returns the read/write access mode of the RenderBuffer
        *
        * @returns RenderBuffer access mode
        */
        [[nodiscard]] RAMSES_API ERenderBufferAccessMode getAccessMode() const;

        /**
        * @brief Returns the sample count used for MSAA
        *
        * @returns RenderBuffer sample count
        */
        [[nodiscard]] RAMSES_API uint32_t getSampleCount() const;

        /**
        * Stores internal data for implementation specifics of RenderBuffer.
        */
        class RenderBufferImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating RenderBuffer instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for RenderBuffer.
        *
        * @param[in] impl Internal data for implementation specifics of RenderBuffer (sink - instance becomes owner)
        */
        explicit RenderBuffer(std::unique_ptr<RenderBufferImpl> impl);
    };
}

#endif
