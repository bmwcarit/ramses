//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Resource/ResourceBase.h"
#include "Resource/LZ4CompressionUtils.h"
#include "Utils/BinaryOutputStream.h"
#include <city.h>

namespace ramses_internal
{
    void ResourceBase::updateHash() const
    {
        if (!m_data.data() || m_data.size() == 0)
        {
            if (!m_compressedData.data())
            {
                m_hash = ResourceContentHash::Invalid();
            }
        }
        else
        {
            // hash blob
            const char* blobToHash = reinterpret_cast<const char*>(m_data.data());
            const cityhash::uint128 cityHashBlob = cityhash::CityHash128(blobToHash, m_data.size());

            // hash metadata
            BinaryOutputStream metaDataStream(1024);
            metaDataStream << static_cast<UInt32>(m_typeID);
            serializeResourceMetadataToStream(metaDataStream);
            metaDataStream << cityhash::Uint128Low64(cityHashBlob);
            metaDataStream << cityhash::Uint128High64(cityHashBlob);
            const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(reinterpret_cast<const char*>(metaDataStream.getData()), metaDataStream.getSize());

            // store resource type in resource hash highest nibble. assume enough bits left for hash and useful in case of error
            static_assert(EResourceType_NUMBER_OF_ELEMENTS <= 0xF, "Too many resource types");

            m_hash.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
            m_hash.highPart = (cityhash::Uint128High64(cityHashMetadataAndBlob) & 0xFFFFFFFFFFFFFFFLU) | (static_cast<uint64_t>(getTypeID()) << 60LU);
        }
    }

    void ResourceBase::compress(CompressionLevel level) const
    {
        if (level > m_currentCompression &&
            m_data.size() > 1000) // only compress if it pays off
        {
            assert(m_data.data());
            getHash(); // try calculate before uncompressed data is lost
            const auto lz4Level = (level == CompressionLevel::Realtime) ?
                LZ4CompressionUtils::CompressionLevel::Fast :
                LZ4CompressionUtils::CompressionLevel::High;
            m_compressedData = LZ4CompressionUtils::compress(m_data, lz4Level);
            m_currentCompression = level;
        }
    }

    void ResourceBase::decompress() const
    {
        if (!m_data.data())
        {
            assert(m_compressedData.data());
            assert(m_compressedData.size());

            m_data = LZ4CompressionUtils::decompress(m_compressedData, m_uncompressedSize);
        }
    }
}
