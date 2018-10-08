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
        BinaryShaderCacheImpl();
        virtual ~BinaryShaderCacheImpl();

        bool hasBinaryShader(const ramses_internal::ResourceContentHash& effectId) const;
        uint32_t getBinaryShaderSize(const ramses_internal::ResourceContentHash& effectId) const;
        uint32_t getBinaryShaderFormat(const ramses_internal::ResourceContentHash& effectId) const;
        bool shouldBinaryShaderBeCached(const ramses_internal::ResourceContentHash& effectId) const;
        void getBinaryShaderData(const ramses_internal::ResourceContentHash& effectId, uint8_t* buffer, uint32_t bufferSize) const;
        void storeBinaryShader(const ramses_internal::ResourceContentHash& effectId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat);
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
        static void serializeBinaryShader(ramses_internal::IOutputStream& outputStream, const ramses_internal::ResourceContentHash& effectId, const ramses_internal::UInt8Vector& binaryShaderData, uint32_t binaryShaderFormat);
        static bool deserializeBinaryShader(ramses_internal::IInputStream& outputStream, ramses_internal::ResourceContentHash& effectId, ramses_internal::UInt8Vector& binaryShaderData, uint32_t& binaryShaderFormat);
        void clear();

        struct BinaryShader
        {
            ramses_internal::UInt8Vector data;
            uint32_t format;

            BinaryShader();
            BinaryShader(const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat);
        };
        typedef ramses_internal::HashMap<ramses_internal::ResourceContentHash, const BinaryShader*> BinaryShaderTable;

        BinaryShaderTable m_binaryShaders;
    };
}

#endif
