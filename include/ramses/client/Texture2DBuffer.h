//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/SceneObject.h"
#include "ramses/framework/TextureEnums.h"
#include <vector>

namespace ramses
{
    namespace internal
    {
        class Texture2DBufferImpl;
    }

    /**
    * @ingroup CoreAPI
    * @brief The Texture2DBuffer is a mutable texture buffer used to hold texture data with the possibility
    * to perform partial updates. This object _must_ be initialized with data, otherwise the contents of it
    * are not specified (garbage data or black, depending on driver behavior).
    * The number of mipmap levels is user given value and the size of the mipchain is computed
    * according to OpenGL specification (each further mipMap level has half the size of the previous
    * mipMap level). Refer to documentation of glTexStorage2D for more details.
    */
    class RAMSES_API Texture2DBuffer : public SceneObject
    {
    public:
        /**
        * @brief Update a subregion of the data of Texture2DBuffer. The caller is responsible to check that
        * the data has the correct size, i.e. the size of a texel times the number of texels specified in the
        * subregion of the texture face. Returns error if the specified subregion exceeds the size of the
        * target mipmap level.
        *
        * @param mipLevel           The level of the mipMap level which will receive the data. First mipMap is 0, second is 1 and so on
        * @param offsetX            The horizontal texel offset into the texture data
        * @param offsetY            The vertical texel offset into the texture data
        * @param width              The horizontal subregion size in texels
        * @param height             The vertical subregion size in texels
        * @param data               Texel data provided for update. The data must be in row-major order wrt. provided width/height.
        *                           Data will be copied internally and no longer needed after this call returns.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool updateData(size_t mipLevel, uint32_t offsetX, uint32_t offsetY, uint32_t width, uint32_t height, const std::byte* data);

        /**
        * @brief Returns the number of mipmap levels created for the Texture2DBuffer (same as provided in
        * ramses::Scene::createTexture2DBuffer() )
        *
        * @return number of mipmap levels
        */
        [[nodiscard]] size_t getMipLevelCount() const;

        /**
        * @brief Returns the size of a specific mipmap level in texels
        *
        * @param[in] mipLevel   The mipMap level of which the size will be returned
        * @param[out] widthOut  the width of the mipMap level which was specified
        * @param[out] heightOut the height of the mipMap level which was specified
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool getMipLevelSize(size_t mipLevel, uint32_t& widthOut, uint32_t& heightOut) const;

        /**
        * @brief Returns the size of a specific mipmap level in bytes
        *
        * @param[in] mipLevel   The mipMap level of which the size will be returned
        * @return Size of data in bytes for given mip level, 0 if mipLevel invalid
        */
        [[nodiscard]] size_t getMipLevelDataSizeInBytes(size_t mipLevel) const;

        /**
        * @brief Returns the texel format provided at creation
        *
        * @return The texel format provided at creation
        */
        [[nodiscard]] ETextureFormat getTexelFormat() const;

        /**
        * @brief Copies the data of a single mip-level into a user-provided buffer.
        *        The amount of data copied is \c bufferSize or \c getMipLevelDataSizeInBytes,
        *        whichever is smaller.
        *
        * @param[in] mipLevel   The mipMap level of which the data will be returned
        * @param[out] buffer The buffer where the mip data will be copied into
        * @param[in] bufferSize The size of given buffer in bytes
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool getMipLevelData(size_t mipLevel, void* buffer, size_t bufferSize) const;

        /**
         * Get the internal data for implementation specifics of Texture2DBuffer.
         */
        [[nodiscard]] internal::Texture2DBufferImpl& impl();

        /**
         * Get the internal data for implementation specifics of Texture2DBuffer.
         */
        [[nodiscard]] const internal::Texture2DBufferImpl& impl() const;

    protected:
        /**
        * @brief Scene is the factory for creating Texture2DBuffer instances.
        */
        friend class internal::SceneObjectRegistry;

        /**
        * @brief Constructor for Texture2DBuffer.
        *
        * @param[in] impl Internal data for implementation specifics of Texture2DBuffer (sink - instance becomes owner)
        */
        explicit Texture2DBuffer(std::unique_ptr<internal::Texture2DBufferImpl> impl);

        /**
        * Stores internal data for implementation specifics of Texture2DBuffer.
        */
        internal::Texture2DBufferImpl& m_impl;
    };
}
