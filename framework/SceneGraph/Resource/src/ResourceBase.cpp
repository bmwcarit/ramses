//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Resource/ResourceBase.h"
#include "Utils/BinaryOutputStream.h"
#include <city.h>

namespace ramses_internal
{
    void ResourceBase::updateHash() const
    {
        if (!m_data.get() || m_data->size() == 0)
        {
            if (!m_compressedData.get())
            {
                m_hash = ResourceContentHash::Invalid();
            }
        }
        else
        {
            // hash blob
            const char* blobToHash = reinterpret_cast<const char*>(m_data->getRawData());
            const cityhash::uint128 cityHashBlob = cityhash::CityHash128(blobToHash, m_data->size());

            // hash metadata
            BinaryOutputStream metaDataStream(1024);
            metaDataStream << static_cast<UInt32>(m_typeID);
            serializeResourceMetadataToStream(metaDataStream);
            metaDataStream << cityhash::Uint128Low64(cityHashBlob);
            metaDataStream << cityhash::Uint128High64(cityHashBlob);
            const cityhash::uint128 cityHashMetadataAndBlob = cityhash::CityHash128(metaDataStream.getData(), metaDataStream.getSize());

            m_hash.lowPart = cityhash::Uint128Low64(cityHashMetadataAndBlob);
            m_hash.highPart = cityhash::Uint128High64(cityHashMetadataAndBlob);
        }
    }
}
