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
    void BinaryShaderCacheImpl::deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats)
    {
        m_supportedFormats = {supportedFormats, supportedFormats + numSupportedFormats };
    }

    bool BinaryShaderCacheImpl::hasBinaryShader(const ramses_internal::ResourceContentHash& effectId) const
    {
        if (!m_binaryShaders.contains(effectId))
            return false;

        // do not report shader as available if not matching any supported format by device
        return ramses_internal::contains_c(m_supportedFormats, getBinaryShaderFormat(effectId));
    }

    uint32_t BinaryShaderCacheImpl::getBinaryShaderSize(const ramses_internal::ResourceContentHash& effectId) const
    {
        const auto iter = m_binaryShaders.find(effectId);
        return (iter != m_binaryShaders.end() ? uint32_t(iter->value.data.size()) : 0u);
    }

    binaryShaderFormatId_t BinaryShaderCacheImpl::getBinaryShaderFormat(const ramses_internal::ResourceContentHash& effectId) const
    {
        const auto iter = m_binaryShaders.find(effectId);
        return (iter != m_binaryShaders.end() ? binaryShaderFormatId_t{ iter->value.format.getValue() } : binaryShaderFormatId_t{ 0 });
    }

    bool BinaryShaderCacheImpl::shouldBinaryShaderBeCached(const ramses_internal::ResourceContentHash& effectId, ramses_internal::SceneId sceneId) const
    {
        UNUSED(effectId);
        UNUSED(sceneId);
        return true;
    }

    void BinaryShaderCacheImpl::getBinaryShaderData(const ramses_internal::ResourceContentHash& effectId, uint8_t* buffer, uint32_t bufferSize) const
    {
        UNUSED(bufferSize);

        assert(nullptr != buffer);
        assert(bufferSize > 0);

        BinaryShaderTable::ConstIterator iter = m_binaryShaders.find(effectId);
        if (iter == m_binaryShaders.end())
        {
            return;
        }

        const uint32_t dataSize = static_cast<uint32_t>(iter->value.data.size());
        assert(bufferSize >= dataSize);
        ramses_internal::PlatformMemory::Copy(buffer, iter->value.data.data(), dataSize);
    }

    void BinaryShaderCacheImpl::storeBinaryShader(const ramses_internal::ResourceContentHash& effectId, ramses_internal::SceneId sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat)
    {
        UNUSED(sceneId);
        assert(nullptr != binaryShaderData);
        assert(binaryShaderDataSize > 0);

        std::lock_guard<std::mutex> g(m_hashMapLock);
        if (m_binaryShaders.contains(effectId))
            return;

        BinaryShader binaryShader = { {binaryShaderData, binaryShaderData + binaryShaderDataSize}, ramses_internal::BinaryShaderFormatID{ binaryShaderFormat.getValue() } };
        m_binaryShaders.put(effectId, std::move(binaryShader));
    }

    bool BinaryShaderCacheImpl::loadFromFile(std::string_view filePath)
    {
        ramses_internal::File file(filePath);
        if (!file.exists())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: file does not exist: " << filePath);
            return false;
        }

        ramses_internal::BinaryFileInputStream fileInputStream(file);
        if (ramses_internal::EStatus::Ok != fileInputStream.getState())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: failed to load file: " << filePath << " errorstate: " << fileInputStream.getState());
            return false;
        }

        FileHeader fileHeader;

        ramses_internal::UInt actualSize = 0;
        if (!file.getSizeInBytes(actualSize) || actualSize < sizeof(FileHeader))
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

        using ramses_internal::Byte;
        std::vector<Byte> content(contentSize);

        fileInputStream.read(content.data(), contentSize);

        const uint64_t checksum = cityhash::CityHash64(reinterpret_cast<const char*>(content.data()), contentSize);

        if (checksum != fileHeader.checksum)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: Checksum was wrong, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        ramses_internal::BinaryInputStream inputStream(content.data());

        uint32_t numBinaryShaders = 0;
        inputStream >> numBinaryShaders;

        std::lock_guard<std::mutex> g(m_hashMapLock);
        for (uint32_t index = 0; index < numBinaryShaders; index++)
        {
            BinaryShader binaryShader;
            ramses_internal::ResourceContentHash effectId;
            if (!deserializeBinaryShader(inputStream, effectId, binaryShader.data, binaryShader.format))
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: Deserialization failed, abort loading at " << index << " of " << numBinaryShaders);
                return false;
            }

            m_binaryShaders.put(effectId, std::move(binaryShader));
        }

        return true;
    }

    void BinaryShaderCacheImpl::saveToFile(std::string_view filePath) const
    {
        ramses_internal::BinaryOutputStream outputStream;

        {
            std::lock_guard<std::mutex> g(m_hashMapLock);
            outputStream << static_cast<uint32_t>(m_binaryShaders.size());

            for (const auto& binaryShader : m_binaryShaders)
                serializeBinaryShader(outputStream, binaryShader.key, binaryShader.value.data, binaryShader.value.format);
        }

        const uint32_t contentSize = static_cast<uint32_t>(outputStream.getSize());
        const uint64_t checksum = cityhash::CityHash64(reinterpret_cast<const char*>(outputStream.getData()), contentSize);

        FileHeader fileHeader = {};
        fileHeader.fileSize         = static_cast<uint32_t>(sizeof(FileHeader)) + contentSize;
        fileHeader.transportVersion = RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        fileHeader.checksum         = checksum;

        ramses_internal::File                   file(filePath);
        ramses_internal::BinaryFileOutputStream outputFileStream(file);

        if (outputFileStream.getState() == ramses_internal::EStatus::Ok)
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

    bool BinaryShaderCacheImpl::deserializeBinaryShader(ramses_internal::IInputStream& inputStream, ramses_internal::ResourceContentHash& effectId, ramses_internal::UInt8Vector& binaryShaderData, ramses_internal::BinaryShaderFormatID& binaryShaderFormat)
    {
        binaryShaderData.clear();
        binaryShaderFormat = {};

        inputStream >> effectId;

        uint32_t binaryShaderSize = 0;
        inputStream >> binaryShaderSize;
        if (0 == binaryShaderSize)
        {
            return false;
        }

        inputStream >> binaryShaderFormat.getReference();

        uint32_t reservedField = 0;
        for (size_t byteIndex = 0; byteIndex < NUM_RESERVED_BYTES / sizeof(uint32_t); byteIndex++)
        {
            inputStream >> reservedField;
        }

        binaryShaderData.resize(binaryShaderSize);
        inputStream.read(reinterpret_cast<char*>(&binaryShaderData.front()), binaryShaderSize);
        return true;
    }

    void BinaryShaderCacheImpl::serializeBinaryShader(ramses_internal::IOutputStream& outputStream, const ramses_internal::ResourceContentHash& effectId, const ramses_internal::UInt8Vector& binaryShaderData, ramses_internal::BinaryShaderFormatID binaryShaderFormat)
    {
        outputStream << effectId;

        const uint32_t binaryShaderDataSize = static_cast<uint32_t>(binaryShaderData.size());
        outputStream << binaryShaderDataSize;
        outputStream << binaryShaderFormat.getValue();

        uint32_t reservedField = 0;
        for (size_t byteIndex = 0; byteIndex < NUM_RESERVED_BYTES / sizeof(uint32_t); byteIndex++)
        {
            outputStream << reservedField;
        }

        outputStream.write(&binaryShaderData.front(), binaryShaderDataSize);
    }
}
