//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IResource.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"

#include <cstdint>
#include <mutex>
#include <string_view>

namespace ramses::internal
{
    class ResourceBase : public IResource
    {
    public:
        explicit ResourceBase(EResourceType typeID, std::string_view name)
            : m_typeID(typeID)
            , m_name(name)
        {
        }

        EResourceType getTypeID() const final override
        {
            return m_typeID;
        }

        const ResourceBlob& getResourceData() const final override
        {
            assert(m_data.data());
            return m_data;
        }

        const CompressedResourceBlob& getCompressedResourceData() const final override
        {
            assert(m_compressedData.data());
            return m_compressedData;
        }

        void setResourceData(ResourceBlob data) final override
        {
            assert(data.size() > 0);
            m_data = std::move(data);
            m_uncompressedSize = static_cast<uint32_t>(m_data.size());
            m_compressedData = CompressedResourceBlob();
            m_currentCompression = CompressionLevel::None;
            m_hash = ResourceContentHash::Invalid();
        }

        void setResourceData(ResourceBlob data, const ResourceContentHash& hash) final override
        {
            assert(data.size() > 0);
            m_data = std::move(data);
            m_uncompressedSize = static_cast<uint32_t>(m_data.size());
            m_compressedData = CompressedResourceBlob();
            m_currentCompression = CompressionLevel::None;
            m_hash = hash;
        }

        void setCompressedResourceData(CompressedResourceBlob compressedData, CompressionLevel compressionLevel, uint32_t uncompressedSize, const ResourceContentHash& hash) final override
        {
            assert(compressedData.size() > 0);
            assert(uncompressedSize > 0);
            m_data = ResourceBlob();
            m_compressedData = std::move(compressedData);
            m_currentCompression = compressionLevel;
            m_uncompressedSize = uncompressedSize;
            m_hash = hash;
        }

        uint32_t getDecompressedDataSize() const override
        {
            return m_uncompressedSize;
        }

        uint32_t getCompressedDataSize() const override
        {
            std::unique_lock<std::mutex> l(m_compressionLock);
            if (m_compressedData.data())
                return static_cast<uint32_t>(m_compressedData.size());
            // 0 == not compressed
            return 0;
        }

        const ResourceContentHash& getHash() const override
        {
            if (!m_hash.isValid())
                updateHash();
            return m_hash;
        }

        void compress(CompressionLevel level) const final override;

        void decompress() const final override;

        bool isCompressedAvailable() const final override
        {
            std::unique_lock<std::mutex> l(m_compressionLock);
            return m_compressedData.data() != nullptr;
        }

        bool isDeCompressedAvailable() const final override
        {
            std::unique_lock<std::mutex> l(m_compressionLock);
            return m_data.data() != nullptr;
        }

        const std::string& getName() const final override
        {
            return m_name;
        }

    protected:
        void setHash(ResourceContentHash hash) const
        {
            m_hash = hash;
        }

        void updateHash() const;

    private:
        const EResourceType m_typeID;
        mutable ResourceBlob m_data;
        mutable CompressedResourceBlob m_compressedData;
        mutable CompressionLevel m_currentCompression = CompressionLevel::None;
        mutable ResourceContentHash m_hash;
        uint32_t m_uncompressedSize = 0;
        std::string m_name;
        mutable std::mutex m_compressionLock;
    };
}
