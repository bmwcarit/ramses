//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DefaultRendererResourceCacheImpl.h"
#include "Utils/File.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/Adler32Checksum.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include <cassert>

namespace ramses
{
    class IDataFunctor
    {
    public:
        virtual ~IDataFunctor() {}
        virtual void addData(const void* data, uint32_t size) = 0;
    };

    class ChecksumDataFunctor : public IDataFunctor
    {
    public:
        ChecksumDataFunctor()
            : m_totalSize(0)
        {
        }

        void addData(const void* data, uint32_t size) override
        {
            m_checksum.addData(reinterpret_cast<const uint8_t*>(data), size);
            m_totalSize += size;
        }

        ramses_internal::Adler32Checksum m_checksum;
        uint32_t                         m_totalSize;
    };

    class SaveDataFunctor : public IDataFunctor
    {
    public:
        explicit SaveDataFunctor(ramses_internal::IOutputStream& outputStream)
            : m_outputStream(outputStream)
        {
        }

        void addData(const void* data, uint32_t size) override
        {
            m_outputStream.write(data, size);
        }

        ramses_internal::IOutputStream& m_outputStream;
    };

    DefaultRendererResourceCacheImpl::DefaultRendererResourceCacheImpl(uint32_t maxCacheSizeInBytes)
        : m_maxCacheSizeInBytes(maxCacheSizeInBytes)
        , m_currentCacheSizeInBytes(0)
    {
        assert(maxCacheSizeInBytes > 0);
    }

    DefaultRendererResourceCacheImpl::~DefaultRendererResourceCacheImpl()
    {
        clear();
    }

    void DefaultRendererResourceCacheImpl::clear()
    {
        m_resourceData.clear();
        m_currentCacheSizeInBytes = 0;
    }

    bool DefaultRendererResourceCacheImpl::hasResource(rendererResourceId_t resourceId, uint32_t& size) const
    {
        for (const auto &it : m_resourceData)
        {
            if (it.resourceId == resourceId)
            {
                size = static_cast<uint32_t>(it.data.size());
                return true;
            }
        }

        size = 0;
        return false;
    }

    bool DefaultRendererResourceCacheImpl::getResourceData(rendererResourceId_t resourceId, uint8_t* buffer, uint32_t bufferSize) const
    {
        for (const auto &it : m_resourceData)
        {
            if (it.resourceId == resourceId)
            {
                if (bufferSize < it.data.size())
                {
                    return false;
                }

                ramses_internal::PlatformMemory::Copy(buffer, it.data.data(), it.data.size());
                return true;
            }
        }

        assert(false);
        return false;
    }

    bool DefaultRendererResourceCacheImpl::shouldResourceBeCached(rendererResourceId_t resourceId, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) const
    {
        UNUSED(resourceId);
        UNUSED(sceneId);

        // This is the place to put an algorithm for deciding if something should be cached or not.

        if (resourceDataSize > m_maxCacheSizeInBytes)
        {
            return false;
        }

        return cacheFlag.getValue() != ResourceCacheFlag_DoNotCache.getValue();
    }

    void DefaultRendererResourceCacheImpl::storeResource(rendererResourceId_t resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId)
    {
        UNUSED(cacheFlag);
        UNUSED(sceneId);

        ByteVector data;
        data.resize(resourceDataSize);
        ramses_internal::PlatformMemory::Copy(data.data(), resourceData, resourceDataSize);

        const bool storingSuccessful = storeResourceInternal(resourceId, data);
        assert(storingSuccessful);
        UNUSED(storingSuccessful);
    }

    bool DefaultRendererResourceCacheImpl::storeResourceInternal(rendererResourceId_t resourceId, ByteVector& data)
    {
        uint32_t tempSize;
        if (hasResource(resourceId, tempSize))
        {
            return false;
        }
        UNUSED(tempSize);

        if (data.size() > m_maxCacheSizeInBytes || data.empty())
        {
            return false;
        }

        makeSpaceForNewItem(static_cast<uint32_t>(data.size()));
        if (m_currentCacheSizeInBytes + data.size() > m_maxCacheSizeInBytes)
        {
            return false;
        }

        ResourceData newData;
        newData.data.swap(data);
        newData.resourceId = resourceId;

        m_resourceData.push_back(newData);
        m_currentCacheSizeInBytes += static_cast<uint32_t>(newData.data.size());

        return m_currentCacheSizeInBytes <= m_maxCacheSizeInBytes;
    }

    void DefaultRendererResourceCacheImpl::makeSpaceForNewItem(uint32_t newItemSizeInBytes)
    {
        assert(newItemSizeInBytes <= m_maxCacheSizeInBytes);

        while (m_currentCacheSizeInBytes + newItemSizeInBytes > m_maxCacheSizeInBytes)
        {
            removeOldestItem();
        }
    }

    void DefaultRendererResourceCacheImpl::removeOldestItem()
    {
        assert(!m_resourceData.empty());
        m_currentCacheSizeInBytes -= static_cast<uint32_t>(m_resourceData[0].data.size());
        m_resourceData.pop_front();
    }

    void DefaultRendererResourceCacheImpl::iterateDataToSave(IDataFunctor& functor) const
    {
        const uint32_t numberOfEntries = static_cast<uint32_t>(m_resourceData.size());
        functor.addData(&numberOfEntries, sizeof(numberOfEntries));

        for (const auto &it : m_resourceData)
        {
            const uint32_t dataSize = static_cast<uint32_t>(it.data.size());

            functor.addData(&it.resourceId, sizeof(it.resourceId));
            functor.addData(&dataSize, sizeof(dataSize));
            functor.addData(it.data.data(), dataSize);
        }
    }

    void DefaultRendererResourceCacheImpl::saveToFile(const char* filePath) const
    {
        ramses_internal::File file(filePath);
        ramses_internal::BinaryFileOutputStream outputStream(file);

        if (outputStream.getState() != ramses_internal::EStatus::Ok)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "BinaryShaderCacheImpl::saveToFile: Failed to open for writing " << filePath);
            file.close();
            return;
        }

        ChecksumDataFunctor checksumFunctor;
        iterateDataToSave(checksumFunctor);

        FileHeader header       = {};
        header.fileSize         = static_cast<uint32_t>(checksumFunctor.m_totalSize + sizeof(header));
        header.transportVersion = RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        header.checksum         = checksumFunctor.m_checksum.getResult();

        outputStream.write(&header, sizeof(header));

        SaveDataFunctor saveFunctor(outputStream);
        iterateDataToSave(saveFunctor);
    }

    bool DefaultRendererResourceCacheImpl::loadFromFile(const char* filePath)
    {
        clear();

        ramses_internal::File file(filePath);

        if (!file.exists())
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DefaultRendererResourceCacheImpl::loadFromFile: file does not exist: " << filePath);
            return false;
        }

        ramses_internal::BinaryFileInputStream inputStream(file);
        if (inputStream.getState() != ramses_internal::EStatus::Ok)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DefaultRendererResourceCacheImpl::loadFromFile: failed to load file: " << filePath << " errorstate: " << inputStream.getState());
            return false;
        }

        FileHeader fileHeader;

        ramses_internal::UInt actualFileSize = 0;
        if (!file.getSizeInBytes(actualFileSize) || actualFileSize < sizeof(FileHeader))
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "DefaultRendererResourceCacheImpl::loadFromFile: Invalid file size, file is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        inputStream >> fileHeader.fileSize;

        if (actualFileSize != fileHeader.fileSize)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "DefaultRendererResourceCacheImpl::loadFromFile: File did not match the size stored in the file header, file "
                     "is corrupt - cache needs to be repopulated and saved again");
            return false;
        }

        inputStream >> fileHeader.transportVersion;

        if (fileHeader.transportVersion != RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "DefaultRendererResourceCacheImpl::loadFromFile: File version "
                         << fileHeader.transportVersion << " did not match the program version "
                         << RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR
                         << " - cache needs to be repopulated and saved again");
            return false;
        }

        ramses_internal::Adler32Checksum calculatedChecksum;

        inputStream >> fileHeader.checksum;

        uint32_t itemCount = 0;
        inputStream >> itemCount;
        calculatedChecksum.addData(reinterpret_cast<ramses_internal::Byte*>(&itemCount), sizeof(itemCount));

        for (uint32_t i = 0; i < itemCount; i++)
        {
            uint64_t resIdLowPart = 0;
            inputStream >> resIdLowPart;
            calculatedChecksum.addData(reinterpret_cast<ramses_internal::Byte*>(&resIdLowPart), sizeof(resIdLowPart));

            uint64_t resIdHighPart = 0;
            inputStream >> resIdHighPart;
            calculatedChecksum.addData(reinterpret_cast<ramses_internal::Byte*>(&resIdHighPart), sizeof(resIdHighPart));

            uint32_t resourceSize = 0;
            inputStream >> resourceSize;
            calculatedChecksum.addData(reinterpret_cast<ramses_internal::Byte*>(&resourceSize), sizeof(resourceSize));

            if (resourceSize > actualFileSize)
            {
                // resourceSize is larger than whole file size, so it must be wrong
                LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                    "DefaultRendererResourceCacheImpl::loadFromFile: Wrong resourceSize: " << resourceSize <<
                    ", file is corrupt - cache needs to be repopulated and saved again");
                clear();
                return false;
            }

            ByteVector data;
            data.resize(resourceSize);
            inputStream.read(reinterpret_cast<ramses_internal::Char*>(data.data()), resourceSize);
            calculatedChecksum.addData(data.data(), resourceSize);

            if (!storeResourceInternal(rendererResourceId_t(resIdLowPart, resIdHighPart), data))
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DefaultRendererResourceCacheImpl::loadFromFile: storeResourceInternal failed"
                                << ", either file is corrupt or cache size too small  - cache needs to be repopulated and saved again");
                clear();
                return false;
            }

            if (inputStream.getState() != ramses_internal::EStatus::Ok)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                         "DefaultRendererResourceCacheImpl::loadFromFile: Reading failed, file is corrupt - cache "
                         "needs to be repopulated and saved again");
                clear();
                return false;
            }
        }

        if (calculatedChecksum.getResult() != fileHeader.checksum)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER,
                     "DefaultRendererResourceCacheImpl::loadFromFile: Checksum was wrong, file is corrupt - cache "
                     "needs to be repopulated and saved again");
            clear();
            return false;
        }

        return true;
    }
}
