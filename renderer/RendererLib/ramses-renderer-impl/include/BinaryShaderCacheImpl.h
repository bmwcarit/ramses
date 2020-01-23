//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYSHADERCACHEIMPL_H
#define RAMSES_BINARYSHADERCACHEIMPL_H

#include "Collections/HashMap.h"
#include "RendererAPI/Types.h"
#include "ramses-renderer-api/Types.h"
#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class IOutputStream;
    class IInputStream;
}

namespace ramses
{
    class BinaryShaderCacheImpl
    {
    public:
        void deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats);
        bool hasBinaryShader(const ramses_internal::ResourceContentHash& effectId) const;
        uint32_t getBinaryShaderSize(const ramses_internal::ResourceContentHash& effectId) const;
        binaryShaderFormatId_t getBinaryShaderFormat(const ramses_internal::ResourceContentHash& effectId) const;
        bool shouldBinaryShaderBeCached(const ramses_internal::ResourceContentHash& effectId, ramses_internal::SceneId sceneId) const;
        void getBinaryShaderData(const ramses_internal::ResourceContentHash& effectId, uint8_t* buffer, uint32_t bufferSize) const;
        void storeBinaryShader(const ramses_internal::ResourceContentHash& effectId, ramses_internal::SceneId sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat);
        void binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const;

        void saveToFile(const char* filePath) const;
        bool loadFromFile(const char* filePath);

        struct FileHeader
        {
            uint32_t fileSize;
            uint32_t transportVersion;
            uint64_t checksum;
        };

    private:
        static void serializeBinaryShader(ramses_internal::IOutputStream& outputStream, const ramses_internal::ResourceContentHash& effectId, const ramses_internal::UInt8Vector& binaryShaderData, ramses_internal::BinaryShaderFormatID binaryShaderFormat);
        static bool deserializeBinaryShader(ramses_internal::IInputStream& outputStream, ramses_internal::ResourceContentHash& effectId, ramses_internal::UInt8Vector& binaryShaderData, ramses_internal::BinaryShaderFormatID& binaryShaderFormat);

        struct BinaryShader
        {
            ramses_internal::UInt8Vector data;
            ramses_internal::BinaryShaderFormatID format;
        };
        typedef ramses_internal::HashMap<ramses_internal::ResourceContentHash, BinaryShader> BinaryShaderTable;

        BinaryShaderTable m_binaryShaders;
        std::vector<binaryShaderFormatId_t> m_supportedFormats;
    };
}

#endif
