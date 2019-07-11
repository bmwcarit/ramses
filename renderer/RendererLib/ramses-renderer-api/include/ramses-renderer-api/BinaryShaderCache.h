//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_BINARYSHADERCACHE_H
#define RAMSES_RENDERERAPI_BINARYSHADERCACHE_H

#include "ramses-renderer-api/IBinaryShaderCache.h"

namespace ramses
{
    /**
     * @brief Provide default implementation for IBinaryShaderCache interface, to be used in RamsesRenderer by setting in RendererConfig.
     *        Also provides functions to serialize binary shaders to file and deserialize binary shaders from file.
     */
    class RAMSES_API BinaryShaderCache : public IBinaryShaderCache
    {
    public:

        /**
         * @brief Default Constructor
         */
        BinaryShaderCache();

        /**
        * @brief Destructor of IBinaryShaderCache
        */
        virtual ~BinaryShaderCache() override;

        /**
        * @brief Check if the cache contains the binary shader for the Effect with the given Effect Id.
        * @param effectId Effect Id of the Effect
        * @return true if there is binary shader for the Effect Id
        *         false otherwise
        */
        virtual bool hasBinaryShader(effectId_t effectId) const override;

        /**
        * @brief Get the binary shader size in bytes for the Effect with the given Effect Id.
        * @param effectId Effect Id of the Effect
        * @return size in bytes for the Effect with the given Effect Id
        *         0 if there is no binary shader in the cache for the Effect with the Effect Id.
        */
        virtual uint32_t getBinaryShaderSize(effectId_t effectId) const override;

        /**
        * @brief Get the binary shader format for the Effect with the given Effect Id.
        * @param effectId Effect Id of the Effect
        * @return binary shader format for the Effect with the given Effect Id.
        *         0 if there is no binary shader in the cache for the Effect with the Effect Id.
        */
        virtual uint32_t getBinaryShaderFormat(effectId_t effectId) const override;

        /**
         * @brief      Returns for a specific effect whethe it should be cached.
         *
         *             This method is called if the shader is not cached yet but could be provided by the renderer.
         *             The cache can decide whether it the shader should be cached.
         * @param[in]  effectId  Effect Id of the Effect
         * @return     true if the effect should be cached, false otherwise.
         */
        virtual bool shouldBinaryShaderBeCached(effectId_t effectId) const override;

        /**
        * @brief Get the binary shader data for the Effect with the given Effect Id.
        * @param effectId Effect Id of the Effect
        * @param buffer pointer to the buffer to get the binary shader data
        * @param bufferSize the size of the buffer to get the binary shader data
        */
        virtual void getBinaryShaderData(effectId_t effectId, uint8_t* buffer, uint32_t bufferSize) const override;

        /**
        * @brief Store the binary shader into the cache.
        * @param effectId Effect Id of the Effect
        * @param binaryShaderData pointer to the binary shader data
        * @param binaryShaderDataSize size of the binary shader data
        * @param binaryShaderFormat format of the binary shader
        */
        virtual void storeBinaryShader(effectId_t effectId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat) override;

        /**
         * @brief Save all binary shaders in the cache to the file with the given path
         * @param filePath the path of the file to save binary shaders
         */
        void saveToFile(const char* filePath) const;

        /**
        * @brief Load all binary shaders from the file with the given path
        * @param filePath the path of the file to load binary shaders
        * @return true when succeed in loading binary shaders from the file
        *         false when fail to load binary shaders from the file
        */
        bool loadFromFile(const char* filePath);

        /**
        * @brief Used by RamsesRenderer to provide a callback with information on the result of a binary shader upload operation.
        *
        *  This method is called by the renderer after each attempted upload operation from a binary shader cache.
        *  It is intended for informational/debugging purposes in detecting an invalid binary shader cache.
        *
        * @param effectId Effect Id of the Effect
        * @param success The result
        */
        virtual void binaryShaderUploaded(effectId_t effectId, bool success) const override;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        BinaryShaderCache(const BinaryShaderCache& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        BinaryShaderCache& operator=(const BinaryShaderCache& other) = delete;

        /**
        * Stores internal data for implementation specifics of this class.
        */
        class BinaryShaderCacheImpl& impl;
    };
}

#endif
