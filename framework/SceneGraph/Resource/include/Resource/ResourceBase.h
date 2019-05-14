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

        virtual const SceneResourceData& getResourceData() const final override
        {
            assert(m_data.get() != 0);
            return m_data;
        }

        virtual const CompressedSceneResourceData& getCompressedResourceData() const final override
        {
            assert(m_compressedData.get() != 0);
            return m_compressedData;
        }

        virtual void setResourceData(const SceneResourceData& data) final override
        {
            m_data = data;
            m_compressedData.reset();
            m_hash = ResourceContentHash::Invalid();
        }

        virtual void setResourceData(const SceneResourceData& data, const ResourceContentHash& hash) final override
        {
            m_data = data;
            m_compressedData.reset();
            m_hash = hash;
        }

        virtual void setCompressedResourceData(const CompressedSceneResourceData& compressedData, const ResourceContentHash& hash) final override
        {
            m_data.reset();
            m_compressedData = compressedData;
            m_hash = hash;
        }

        virtual UInt32 getDecompressedDataSize() const override
        {
            if (isCompressedAvailable())
            {
                return getCompressedResourceData()->getDecompressedSize();
            }
            else
            {
                return getResourceData()->size();
            }
        }

        virtual UInt32 getCompressedDataSize() const override
        {
            if (isCompressedAvailable())
            {
                return getCompressedResourceData()->size();
            }
            else
            {
                // 0 == not compressed
                return 0u;
            }
        }

        virtual const ResourceContentHash& getHash() const override
        {
            if (!m_hash.isValid())
            {
                updateHash();
            }
            return m_hash;
        }

        void compress(CompressionLevel level) const final override
        {
            if (level != CompressionLevel::NONE &&
                isCompressable() &&
                !isCompressedAvailable() &&
                m_data->size() > 1000) // only compress if it pays off
            {
                assert(m_data.get() != 0);
                getHash(); // try calculate before uncompressed data is lost
                const auto lz4Level = (level == CompressionLevel::REALTIME) ?
                    LZ4CompressionUtils::CompressionLevel::Fast :
                    LZ4CompressionUtils::CompressionLevel::High;
                m_compressedData = CompressedSceneResourceData(new CompressedMemoryBlob(*m_data.get(), lz4Level));
            }
        }

        void decompress() const final override
        {
            if (m_data.get() == 0)
            {
                assert(m_compressedData.get() != 0);
                m_data = SceneResourceData(new MemoryBlob(*m_compressedData.get()));
            }
        }

        Bool isCompressedAvailable() const final override
        {
            return m_compressedData.get() != 0;
        }

        Bool isDeCompressedAvailable() const final override
        {
            return m_data.get() != 0;
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
        mutable SceneResourceData m_data;
        mutable CompressedSceneResourceData m_compressedData;
        mutable ResourceContentHash m_hash;
        ResourceCacheFlag m_cacheFlag;
        String m_name;
    };
}

#endif
