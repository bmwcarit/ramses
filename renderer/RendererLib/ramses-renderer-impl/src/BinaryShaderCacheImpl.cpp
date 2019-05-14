//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "BinaryShaderCacheImpl.h"
#include "Utils/File.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "city.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"

namespace
{
    const ramses_internal::UInt32 NUM_RESERVED_BYTES = 32;
}

namespace ramses
{
    BinaryShaderCacheImpl::BinaryShader::BinaryShader()
    : format(0)
    {

    }

    BinaryShaderCacheImpl::BinaryShader::BinaryShader(const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat)
    : data(binaryShaderDataSize)
    , format(binaryShaderFormat)
    {
        ramses_internal::PlatformMemory::Copy(&data.front(), binaryShaderData, binaryShaderDataSize);
    }

    BinaryShaderCacheImpl::BinaryShaderCacheImpl()
    {

    }

    BinaryShaderCacheImpl::~BinaryShaderCacheImpl()
    {
        clear();
    }

    bool BinaryShaderCacheImpl::hasBinaryShader(const ramses_internal::ResourceContentHash& effectId) const
    {
        return m_binaryShaders.contains(effectId);
    }

    uint32_t BinaryShaderCacheImpl::getBinaryShaderSize(const ramses_internal::ResourceContentHash& effectId) const
    {
        BinaryShaderTable::Iterator iter = m_binaryShaders.find(effectId);
        if (iter == m_binaryShaders.end())
        {
            return 0;
        }

        return static_cast<uint32_t>(iter->value->data.size());
    }

    uint32_t BinaryShaderCacheImpl::getBinaryShaderFormat(const ramses_internal::ResourceContentHash& effectId) const
    {
        BinaryShaderTable::Iterator iter = m_binaryShaders.find(effectId);
        if (iter == m_binaryShaders.end())
        {
            return 0;
        }

        return iter->value->format;
    }

    bool BinaryShaderCacheImpl::shouldBinaryShaderBeCached(const ramses_internal::ResourceContentHash& effectId) const
    {
        UNUSED(effectId);
        return true;
    }

    void BinaryShaderCacheImpl::getBinaryShaderData(const ramses_internal::ResourceContentHash& effectId, uint8_t* buffer, uint32_t bufferSize) const
    {
        UNUSED(bufferSize);

        assert(NULL != buffer);
        assert(bufferSize > 0);

        BinaryShaderTable::Iterator iter = m_binaryShaders.find(effectId);
        if (iter == m_binaryShaders.end())
        {
            return;
        }

        const uint32_t dataSize = static_cast<uint32_t>(iter->value->data.size());
        assert(bufferSize >= dataSize);
        ramses_internal::PlatformMemory::Copy(buffer, &iter->value->data.front(), dataSize);
    }

    void BinaryShaderCacheImpl::storeBinaryShader(const ramses_internal::ResourceContentHash& effectId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat)
    {
        assert(NULL != binaryShaderData);
        assert(binaryShaderDataSize > 0);

        if (hasBinaryShader(effectId))
        {
            assert(false);
            return;
        }

        BinaryShader* binaryShader = new BinaryShader(binaryShaderData, binaryShaderDataSize, binaryShaderFormat);
        m_binaryShaders.put(effectId, binaryShader);
    }

    void BinaryShaderCacheImpl::clear()
    {
        for(const auto& binaryShader : m_binaryShaders)
        {
            const BinaryShader* shader = binaryShader.value;
            delete shader;
        }

        m_binaryShaders.clear();
    }

    bool BinaryShaderCacheImpl::loadFromFile(const char* filePath)
    {
        ramses_internal::File file(filePath);
        if (!file.exists())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: file does not exist: " << filePath);
            return false;
        }

        ramses_internal::BinaryFileInputStream fileInputStream(file);
        if (ramses_internal::EStatus_RAMSES_OK != fileInputStream.getState())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: failed to load file: " << filePath << " errorstate: " << fileInputStream.getState());
            return false;
        }

        FileHeader fileHeader;

        ramses_internal::UInt actualSize = 0;
        if (file.getSizeInBytes(actualSize) != ramses_internal::EStatus_RAMSES_OK || actualSize < sizeof(FileHeader))
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: Invalid file size - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.fileSize;

        if (actualSize != fileHeader.fileSize)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: File did not match the size stored in the file header, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.transportVersion;

        if (fileHeader.transportVersion != RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: File version " << fileHeader.transportVersion << " did not match the program version " << RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR << " - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.checksum;

        const uint32_t contentSize = fileHeader.fileSize - sizeof(fileHeader);

        std::vector<uint8_t> content(contentSize);
        ramses_internal::Char* contentData = reinterpret_cast<ramses_internal::Char*>(content.data());

        fileInputStream.read(contentData, contentSize);

        const uint64_t checksum = cityhash::CityHash64(contentData, contentSize);

        if (checksum != fileHeader.checksum)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: Checksum was wrong, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        ramses_internal::BinaryInputStream inputStream(contentData);

        uint32_t numBinaryShaders = 0;
        inputStream >> numBinaryShaders;

        for (uint32_t index = 0; index < numBinaryShaders; index++)
        {
            BinaryShader* binaryShader = new BinaryShader();

            ramses_internal::ResourceContentHash effectId;
            if (!deserializeBinaryShader(inputStream, effectId, binaryShader->data, binaryShader->format))
            {
                delete binaryShader;
                return false;
            };

            m_binaryShaders.put(effectId, binaryShader);
        }

        return true;
    }

    void BinaryShaderCacheImpl::saveToFile(const char* filePath) const
    {
        ramses_internal::BinaryOutputStream outputStream;

        outputStream << static_cast<uint32_t>(m_binaryShaders.count());

        for (const auto& binaryShader : m_binaryShaders)
        {
            const ramses_internal::ResourceContentHash& effectId    = binaryShader.key;
            const BinaryShader*                         shader      = binaryShader.value;
            serializeBinaryShader(outputStream, effectId, shader->data, shader->format);
        }

        const uint32_t contentSize = outputStream.getSize();
        const uint64_t checksum = cityhash::CityHash64(outputStream.getData(), contentSize);

        FileHeader fileHeader = {};
        fileHeader.fileSize         = static_cast<uint32_t>(sizeof(FileHeader)) + contentSize;
        fileHeader.transportVersion = RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        fileHeader.checksum         = checksum;

        ramses_internal::File                   file(filePath);
        ramses_internal::BinaryFileOutputStream outputFileStream(file);

        if (outputFileStream.getState() == ramses_internal::EStatus_RAMSES_OK)
        {
            outputFileStream << fileHeader.fileSize << fileHeader.transportVersion << fileHeader.checksum;
            outputFileStream.write(outputStream.getData(), contentSize);
        }
        else
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::saveToFile: failed to open " << filePath);
        }
    }

    void BinaryShaderCacheImpl::binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const
    {
        if (!success)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCache: Failed to upload binary shader from cache for effect id: " << effectHash);
        }
    }

    bool BinaryShaderCacheImpl::deserializeBinaryShader(ramses_internal::IInputStream& inputStream, ramses_internal::ResourceContentHash& effectId, ramses_internal::UInt8Vector& binaryShaderData, uint32_t& binaryShaderFormat)
    {
        binaryShaderData.clear();
        binaryShaderFormat = 0;

        inputStream >> effectId;

        uint32_t binaryShaderSize = 0;
        inputStream >> binaryShaderSize;
        if (0 == binaryShaderSize)
        {
            return false;
        }

        inputStream >> binaryShaderFormat;

        uint32_t reservedField = 0;
        for (uint32_t byteIndex = 0; byteIndex < NUM_RESERVED_BYTES / sizeof(uint32_t); byteIndex++)
        {
            inputStream >> reservedField;
        }

        binaryShaderData.resize(binaryShaderSize);
        inputStream.read(reinterpret_cast<ramses_internal::Char*>(&binaryShaderData.front()), binaryShaderSize);
        return true;
    }

    void BinaryShaderCacheImpl::serializeBinaryShader(ramses_internal::IOutputStream& outputStream, const ramses_internal::ResourceContentHash& effectId, const ramses_internal::UInt8Vector& binaryShaderData, uint32_t binaryShaderFormat)
    {
        outputStream << effectId;

        const uint32_t binaryShaderDataSize = static_cast<uint32_t>(binaryShaderData.size());
        outputStream << binaryShaderDataSize;
        outputStream << binaryShaderFormat;

        uint32_t reservedField = 0;
        for (uint32_t byteIndex = 0; byteIndex < NUM_RESERVED_BYTES / sizeof(uint32_t); byteIndex++)
        {
            outputStream << reservedField;
        }

        outputStream.write(&binaryShaderData.front(), binaryShaderDataSize);
    }
}
