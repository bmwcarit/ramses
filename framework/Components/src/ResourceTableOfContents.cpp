//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceTableOfContents.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Components/InputStreamAdapter.h"
#include "Utils/LogMacros.h"
#include <algorithm>
#include <array>

namespace ramses_internal
{

    Bool ResourceTableOfContents::containsResource(const ResourceContentHash& hash) const
    {
        return m_fileContents.contains(hash);
    }

    void ResourceTableOfContents::registerContents(const ResourceInfo& resourceInfo, UInt32 offsetInBytes, UInt32 sizeInBytes)
    {
        ResourceFileEntry fileEntry;
        fileEntry.offsetInBytes = offsetInBytes;
        fileEntry.sizeInBytes = sizeInBytes;
        fileEntry.resourceInfo = resourceInfo;
        m_fileContents.put(resourceInfo.hash, fileEntry);
    }

    const ResourceFileEntry& ResourceTableOfContents::getEntryForHash(const ResourceContentHash& hash) const
    {
        return *m_fileContents.get(hash);
    }

    void ResourceTableOfContents::writeTOCToStream(IOutputStream& outstream)
    {
        const uint32_t numberOfEntries = static_cast<uint32_t>(m_fileContents.count());
        outstream << numberOfEntries;

        // sort resources to get deterministic file
        std::vector<ResourceFileEntry> vec;
        vec.reserve(numberOfEntries);
        for (const auto& entry : m_fileContents)
        {
            vec.push_back(entry.value);
        }

        std::sort(vec.begin(), vec.end(), [](ResourceFileEntry const& lhs, ResourceFileEntry const& rhs)
        {
            return lhs.resourceInfo.hash < rhs.resourceInfo.hash;
        });

        for (const auto& it : vec)
        {
            outstream << static_cast<UInt32>(it.resourceInfo.type);
            outstream << it.resourceInfo.hash;
            outstream << it.resourceInfo.decompressedSize;
            outstream << it.resourceInfo.compressedSize;
            outstream << it.offsetInBytes;
            outstream << it.sizeInBytes;
        }
    }

    const TableOfContentsMap& ResourceTableOfContents::getFileContents() const
    {
        return m_fileContents;
    }

    Bool ResourceTableOfContents::readTOCPosAndTOCFromStream(BinaryFileInputStream& instream)
    {
        uint32_t numberOfEntries = 0;
        instream >> numberOfEntries;
        std::array<uint32_t, EResourceType_NUMBER_OF_ELEMENTS> objectCounts = {};

        for (uint32_t i = 0; i < numberOfEntries; ++i)
        {
            ResourceInfo info;
            UInt32 typeValue = 0;
            instream >> typeValue;
            info.type = static_cast<EResourceType>(typeValue);
            instream >> info.hash;
            instream >> info.decompressedSize;
            instream >> info.compressedSize;
            UInt32 offsetInBytes = 0;
            instream >> offsetInBytes;
            UInt32 sizeInBytes = 0;
            instream >> sizeInBytes;
            registerContents(info, offsetInBytes, sizeInBytes);
            ++objectCounts[info.type];

            const uint32_t resourceSizeToWarnAboutInBytes = 100 * 1024 * 1024;
            if (info.decompressedSize > resourceSizeToWarnAboutInBytes)
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "ResourceTableOfContents::readTOCPosAndTOCFromStream: Loading a resource that is larger than " << resourceSizeToWarnAboutInBytes << " bytes, hash: " << info.hash << " (" << info.decompressedSize << " bytes)");
            }
        }

        LOG_DEBUG_F(CONTEXT_PROFILING, ([&](StringOutputStream& sos) {
                    sos << "ResourceTableOfContents::readTOCPosAndTOCFromStream: LL resource counts in TOC\n";
                    for (uint32_t i = 0; i < EResourceType_NUMBER_OF_ELEMENTS; i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << EnumToString(static_cast<EResourceType>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        return instream.getState() == EStatus_RAMSES_OK;
    }
}
