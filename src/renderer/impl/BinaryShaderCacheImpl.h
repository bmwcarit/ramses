//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/RendererLib/Types.h"
#include "ramses/renderer/Types.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"

#include <mutex>
#include <string_view>

namespace ramses::internal
{
    class IOutputStream;
    class IInputStream;

    class BinaryShaderCacheImpl
    {
    public:
        void deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats);
        bool hasBinaryShader(const ResourceContentHash& effectId) const;
        uint32_t getBinaryShaderSize(const ResourceContentHash& effectId) const;
        binaryShaderFormatId_t getBinaryShaderFormat(const ResourceContentHash& effectId) const;
        bool shouldBinaryShaderBeCached(const ResourceContentHash& effectId, SceneId sceneId) const;
        void getBinaryShaderData(const ResourceContentHash& effectId, std::byte* buffer, uint32_t bufferSize) const;
        void storeBinaryShader(const ResourceContentHash& effectId, SceneId sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat);
        void binaryShaderUploaded(ResourceContentHash effectHash, bool success) const;

        void saveToFile(std::string_view filePath) const;
        bool loadFromFile(std::string_view filePath);

        struct FileHeader
        {
            uint32_t fileSize;
            uint32_t transportVersion;
            uint64_t checksum;
        };

    private:
        static void serializeBinaryShader(IOutputStream& outputStream, const ResourceContentHash& effectId, const std::vector<std::byte>& binaryShaderData, BinaryShaderFormatID binaryShaderFormat);
        static bool deserializeBinaryShader(IInputStream& inputStream, ResourceContentHash& effectId, std::vector<std::byte>& binaryShaderData, BinaryShaderFormatID& binaryShaderFormat);

        struct BinaryShader
        {
            std::vector<std::byte> data;
            BinaryShaderFormatID format;
        };
        using BinaryShaderTable = HashMap<ResourceContentHash, BinaryShader>;

        BinaryShaderTable m_binaryShaders;
        std::vector<binaryShaderFormatId_t> m_supportedFormats;
        // protects HashMap write of new shaders concurrently with saving of file
        // WARNING: Does not protect loading from file concurrently with querying for shaders!
        // Reason: Avoid performance degradation because unknown if Integrity mutexes are cheap without congestion.
        mutable std::mutex m_hashMapLock;
    };
}
