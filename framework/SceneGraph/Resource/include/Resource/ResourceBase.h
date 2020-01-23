//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEBASE_H
#define RAMSES_RESOURCEBASE_H

#include "IResource.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/IScene.h"

namespace ramses_internal
{
    class ResourceBase : public IResource
    {
    public:
        explicit ResourceBase(EResourceType typeID, ResourceCacheFlag cacheFlag, const String& name)
            : m_typeID(typeID)
            , m_data()
            , m_compressedData()
            , m_hash()
            , m_cacheFlag(cacheFlag)
            , m_name(name)
        {
        }

        virtual EResourceType getTypeID() const final override
        {
            return m_typeID;
        }

        virtual const ResourceBlob& getResourceData() const final override
        {
            assert(m_data.data());
            return m_data;
        }

        virtual const CompressedResouceBlob& getCompressedResourceData() const final override
        {
            assert(m_compressedData.data());
            return m_compressedData;
        }

        virtual void setResourceData(ResourceBlob data) final override
        {
            assert(data.size() > 0);
            m_data = std::move(data);
            m_uncompressedSize = static_cast<uint32_t>(m_data.size());
            m_compressedData = CompressedResouceBlob();
            m_hash = ResourceContentHash::Invalid();
        }

        virtual void setResourceData(ResourceBlob data, const ResourceContentHash& hash) final override
        {
            assert(data.size() > 0);
            m_data = std::move(data);
            m_uncompressedSize = static_cast<uint32_t>(m_data.size());
            m_compressedData = CompressedResouceBlob();
            m_hash = hash;
        }

        virtual void setCompressedResourceData(CompressedResouceBlob compressedData, uint32_t uncompressedSize, const ResourceContentHash& hash) final override
        {
            assert(compressedData.size() > 0);
            assert(uncompressedSize > 0);
            m_data = ResourceBlob();
            m_compressedData = std::move(compressedData);
            m_uncompressedSize = uncompressedSize;
            m_hash = hash;
        }

        virtual UInt32 getDecompressedDataSize() const override
        {
            return m_uncompressedSize;
        }

        virtual UInt32 getCompressedDataSize() const override
        {
            if (m_compressedData.data())
                return static_cast<uint32_t>(m_compressedData.size());
            // 0 == not compressed
            return 0;
        }

        virtual const ResourceContentHash& getHash() const override
        {
            if (!m_hash.isValid())
                updateHash();
            return m_hash;
        }

        void compress(CompressionLevel level) const final override;

        void decompress() const final override;

        bool isCompressedAvailable() const final override
        {
            return m_compressedData.data();
        }

        bool isDeCompressedAvailable() const final override
        {
            return m_data.data();
        }

        ResourceCacheFlag getCacheFlag() const final override
        {
            return m_cacheFlag;
        }

        const String& getName() const final override
        {
            return m_name;
        }

    protected:
        void setHash(ResourceContentHash hash) const
        {
            m_hash = hash;
        }

        virtual bool isCompressable() const
        {
            return true;
        }

        void updateHash() const;

    private:
        const EResourceType m_typeID;
        mutable ResourceBlob m_data;
        mutable CompressedResouceBlob m_compressedData;
        mutable ResourceContentHash m_hash;
        uint32_t m_uncompressedSize;
        ResourceCacheFlag m_cacheFlag;
        String m_name;
    };
}

#endif
