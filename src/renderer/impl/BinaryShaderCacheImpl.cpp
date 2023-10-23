//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/BinaryShaderCacheImpl.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "city.h"
#include "internal/Communication/TransportCommon/RamsesTransportProtocolVersion.h"

namespace
{
    const uint32_t NUM_RESERVED_BYTES = 32;
}

namespace ramses::internal
{
    void BinaryShaderCacheImpl::deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats)
    {
        m_supportedFormats = {supportedFormats, supportedFormats + numSupportedFormats };
    }

    bool BinaryShaderCacheImpl::hasBinaryShader(const ResourceContentHash& effectId) const
    {
        if (!m_binaryShaders.contains(effectId))
            return false;

        // do not report shader as available if not matching any supported format by device
        return contains_c(m_supportedFormats, getBinaryShaderFormat(effectId));
    }

    uint32_t BinaryShaderCacheImpl::getBinaryShaderSize(const ResourceContentHash& effectId) const
    {
        const auto iter = m_binaryShaders.find(effectId);
        return (iter != m_binaryShaders.end() ? uint32_t(iter->value.data.size()) : 0u);
    }

    binaryShaderFormatId_t BinaryShaderCacheImpl::getBinaryShaderFormat(const ResourceContentHash& effectId) const
    {
        const auto iter = m_binaryShaders.find(effectId);
        return (iter != m_binaryShaders.end() ? binaryShaderFormatId_t{ iter->value.format.getValue() } : binaryShaderFormatId_t{ 0 });
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): design decision
    bool BinaryShaderCacheImpl::shouldBinaryShaderBeCached([[maybe_unused]] const ResourceContentHash& effectId,
                                                           [[maybe_unused]] SceneId                    sceneId) const
    {
        return true;
    }

    void BinaryShaderCacheImpl::getBinaryShaderData(const ResourceContentHash& effectId, std::byte* buffer, [[maybe_unused]] uint32_t bufferSize) const
    {
        assert(nullptr != buffer);
        assert(bufferSize > 0);

        BinaryShaderTable::ConstIterator iter = m_binaryShaders.find(effectId);
        if (iter == m_binaryShaders.end())
        {
            return;
        }

        const auto dataSize = static_cast<uint32_t>(iter->value.data.size());
        assert(bufferSize >= dataSize);
        PlatformMemory::Copy(buffer, iter->value.data.data(), dataSize);
    }

    void BinaryShaderCacheImpl::storeBinaryShader(const ResourceContentHash& effectId,
                                                  [[maybe_unused]] SceneId   sceneId,
                                                  const std::byte*           binaryShaderData,
                                                  uint32_t                   binaryShaderDataSize,
                                                  binaryShaderFormatId_t     binaryShaderFormat)
    {
        assert(nullptr != binaryShaderData);
        assert(binaryShaderDataSize > 0);

        std::lock_guard<std::mutex> g(m_hashMapLock);
        if (m_binaryShaders.contains(effectId))
            return;

        BinaryShader binaryShader = { {binaryShaderData, binaryShaderData + binaryShaderDataSize}, BinaryShaderFormatID{ binaryShaderFormat.getValue() } };
        m_binaryShaders.put(effectId, binaryShader);
    }

    bool BinaryShaderCacheImpl::loadFromFile(std::string_view filePath)
    {
        ramses::internal::File file(filePath);
        if (!file.exists())
        {
            LOG_WARN(CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: file does not exist: " << filePath);
            return false;
        }

        BinaryFileInputStream fileInputStream(file);
        if (EStatus::Ok != fileInputStream.getState())
        {
            LOG_WARN(CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: failed to load file: " << filePath << " errorstate: " << fileInputStream.getState());
            return false;
        }

        FileHeader fileHeader{};

        size_t actualSize = 0;
        if (!file.getSizeInBytes(actualSize) || actualSize < sizeof(FileHeader))
        {
            LOG_WARN(CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: Invalid file size - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.fileSize;

        if (actualSize != fileHeader.fileSize)
        {
            LOG_WARN(CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: File did not match the size stored in the file header, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.transportVersion;

        if (fileHeader.transportVersion != RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR)
        {
            LOG_WARN(CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: File version " << fileHeader.transportVersion << " did not match the program version " << RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR << " - cache needs to be repopulated and saved again");
            return false;
        }

        fileInputStream >> fileHeader.checksum;

        const uint32_t contentSize = fileHeader.fileSize - sizeof(fileHeader);

        using std::byte;
        std::vector<std::byte> content(contentSize);

        fileInputStream.read(content.data(), contentSize);

        const uint64_t checksum = cityhash::CityHash64(reinterpret_cast<const char*>(content.data()), contentSize);

        if (checksum != fileHeader.checksum)
        {
            LOG_WARN(CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::loadFromFile: Checksum was wrong, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        BinaryInputStream inputStream(content.data());

        uint32_t numBinaryShaders = 0;
        inputStream >> numBinaryShaders;

        std::lock_guard<std::mutex> g(m_hashMapLock);
        for (uint32_t index = 0; index < numBinaryShaders; index++)
        {
            BinaryShader binaryShader;
            ResourceContentHash effectId;
            if (!deserializeBinaryShader(inputStream, effectId, binaryShader.data, binaryShader.format))
            {
                LOG_WARN(CONTEXT_RENDERER, "BinaryShaderCacheImpl::loadFromFile: Deserialization failed, abort loading at " << index << " of " << numBinaryShaders);
                return false;
            }

            m_binaryShaders.put(effectId, binaryShader);
        }

        return true;
    }

    void BinaryShaderCacheImpl::saveToFile(std::string_view filePath) const
    {
        BinaryOutputStream outputStream;

        {
            std::lock_guard<std::mutex> g(m_hashMapLock);
            outputStream << static_cast<uint32_t>(m_binaryShaders.size());

            for (const auto& binaryShader : m_binaryShaders)
                serializeBinaryShader(outputStream, binaryShader.key, binaryShader.value.data, binaryShader.value.format);
        }

        const auto contentSize = static_cast<uint32_t>(outputStream.getSize());
        const uint64_t checksum = cityhash::CityHash64(reinterpret_cast<const char*>(outputStream.getData()), contentSize);

        FileHeader fileHeader = {};
        fileHeader.fileSize         = static_cast<uint32_t>(sizeof(FileHeader)) + contentSize;
        fileHeader.transportVersion = RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        fileHeader.checksum         = checksum;

        ramses::internal::File                   file(filePath);
        ramses::internal::BinaryFileOutputStream outputFileStream(file);

        if (outputFileStream.getState() == EStatus::Ok)
        {
            outputFileStream << fileHeader.fileSize << fileHeader.transportVersion << fileHeader.checksum;
            outputFileStream.write(outputStream.getData(), contentSize);
        }
        else
        {
            LOG_WARN(CONTEXT_RENDERER,
                     "BinaryShaderCacheImpl::saveToFile: failed to open " << filePath);
        }
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static): design decision
    void BinaryShaderCacheImpl::binaryShaderUploaded(ResourceContentHash effectHash, bool success) const
    {
        if (!success)
        {
            LOG_WARN(CONTEXT_RENDERER, "BinaryShaderCache: Failed to upload binary shader from cache for effect id: " << effectHash);
        }
    }

    bool BinaryShaderCacheImpl::deserializeBinaryShader(IInputStream& inputStream, ResourceContentHash& effectId, std::vector<std::byte>& binaryShaderData, BinaryShaderFormatID& binaryShaderFormat)
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

    void BinaryShaderCacheImpl::serializeBinaryShader(IOutputStream& outputStream, const ResourceContentHash& effectId, const std::vector<std::byte>& binaryShaderData, BinaryShaderFormatID binaryShaderFormat)
    {
        outputStream << effectId;

        const auto binaryShaderDataSize = static_cast<uint32_t>(binaryShaderData.size());
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
