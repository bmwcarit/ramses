//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ResourceTableOfContents.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/Core/Utils/LogMacros.h"
#include <algorithm>
#include <array>

namespace ramses::internal
{

    bool ResourceTableOfContents::containsResource(const ResourceContentHash& hash) const
    {
        return m_fileContents.contains(hash);
    }

    void ResourceTableOfContents::registerContents(const ResourceInfo& resourceInfo, uint32_t offsetInBytes, uint32_t sizeInBytes)
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
        const auto numberOfEntries = static_cast<uint32_t>(m_fileContents.size());
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
            outstream << static_cast<uint32_t>(it.resourceInfo.type);
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

    bool ResourceTableOfContents::readTOCPosAndTOCFromStream(IInputStream& instream)
    {
        uint32_t numberOfEntries = 0;
        instream >> numberOfEntries;
        std::array<uint32_t, EResourceType_NUMBER_OF_ELEMENTS> objectCounts = {};

        for (uint32_t i = 0; i < numberOfEntries; ++i)
        {
            ResourceInfo info;
            uint32_t typeValue = 0;
            instream >> typeValue;
            info.type = static_cast<EResourceType>(typeValue);
            instream >> info.hash;
            instream >> info.decompressedSize;
            instream >> info.compressedSize;
            uint32_t offsetInBytes = 0;
            instream >> offsetInBytes;
            uint32_t sizeInBytes = 0;
            instream >> sizeInBytes;
            registerContents(info, offsetInBytes, sizeInBytes);
            ++objectCounts[static_cast<size_t>(info.type)];

            const uint32_t resourceSizeToWarnAboutInBytes = 100 * 1024 * 1024;
            if (info.decompressedSize > resourceSizeToWarnAboutInBytes)
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "ResourceTableOfContents::readTOCPosAndTOCFromStream: Loading a resource that is larger than {} bytes, hash: {} ({} bytes)", resourceSizeToWarnAboutInBytes, info.hash, info.decompressedSize);
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

        return instream.getState() == EStatus::Ok;
    }
}
