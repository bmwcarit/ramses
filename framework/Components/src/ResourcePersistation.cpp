//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourcePersistation.h"

#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Utils/File.h"
#include "Utils/VoidOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Components/ManagedResource.h"
#include "Components/ResourceTableOfContents.h"
#include "Resource/ResourceInfo.h"
#include "Resource/IResource.h"
#include "Components/SingleResourceSerialization.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void ResourcePersistation::WriteOneResourceToStream(IOutputStream& outStream, const ManagedResource& resource)
    {
        SingleResourceSerialization::SerializeResource(outStream, *resource.get());
    }

    std::unique_ptr<IResource> ResourcePersistation::ReadOneResourceFromStream(IInputStream& inStream, const ResourceContentHash& hash)
    {
        return SingleResourceSerialization::DeserializeResource(inStream, hash);
    }

    void ResourcePersistation::WriteNamedResourcesWithTOCToStream(IOutputStream& outStream, const ManagedResourceVector& resourcesForFile, bool compress)
    {
        // achieve maximum resource file loading speed by reading in increasing file position order
        // so store TOC first followed by all resources, as the toc is read before the resources

        UInt offsetForTOC = 0;
        outStream.getPos(offsetForTOC);

        // get size and offset of resources by writing to dummy stream
        VoidOutputStream dummyStream;
        ResourceTableOfContents dummyToc;
        std::vector<UInt32> resourceOffsetSize;
        resourceOffsetSize.reserve(resourcesForFile.size() * 2);
        UInt32 offsetBeforeWrite = 0;
        UInt32 currentPosAfterWrite = 0;

        // possible compress all resources before writing
        for (const auto& res : resourcesForFile)
        {
            res->compress(compress ? IResource::CompressionLevel::Offline : IResource::CompressionLevel::None);
        }

        for (const auto& res : resourcesForFile)
        {
            WriteOneResourceToStream(dummyStream, res);
            currentPosAfterWrite = static_cast<uint32_t>(dummyStream.getSize());
            const UInt32 bytesWritten = currentPosAfterWrite - offsetBeforeWrite;
            resourceOffsetSize.push_back(offsetBeforeWrite);
            resourceOffsetSize.push_back(bytesWritten);

            dummyToc.registerContents(ResourceInfo(res.get()), 0, 0);

            offsetBeforeWrite = static_cast<UInt32>(currentPosAfterWrite);
        }

        // get size of TOC by writing to dummy stream
        dummyToc.writeTOCToStream(dummyStream);
        currentPosAfterWrite = static_cast<uint32_t>(dummyStream.getSize());
        const UInt32 tocSize = currentPosAfterWrite - offsetBeforeWrite;

        // create final TOC with correct resource offsets
        ResourceTableOfContents toc;
        UInt32 i = 0;
        for (const auto& res : resourcesForFile)
        {
            const UInt32 resourceOffset = resourceOffsetSize[i++];
            const UInt32 resourceSize = resourceOffsetSize[i++];
            toc.registerContents(ResourceInfo(res.get()), static_cast<UInt32>(offsetForTOC) + tocSize + resourceOffset, resourceSize);
        }

        // write final toc and resources to output stream
        toc.writeTOCToStream(outStream);
        for (const auto& res : resourcesForFile)
        {
            WriteOneResourceToStream(outStream, res);
        }
    }

    std::unique_ptr<IResource> ResourcePersistation::RetrieveResourceFromStream(IInputStream& inStream, const ResourceFileEntry& fileEntry)
    {
        LOG_DEBUG_P(CONTEXT_FRAMEWORK, "ResourcePersistation::RetrieveResourceFromStream: Hash {}, Size {}, Offset {}",
                    fileEntry.resourceInfo.hash, fileEntry.sizeInBytes, fileEntry.offsetInBytes);

        if (inStream.seek(fileEntry.offsetInBytes, IInputStream::Seek::FromBeginning) != EStatus::Ok)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "ResourcePersistation::RetrieveResourceFromStream: seek failed for resource {}", fileEntry.resourceInfo.hash);
            return {};
        }

        std::unique_ptr<IResource> resource = ReadOneResourceFromStream(inStream, fileEntry.resourceInfo.hash);
        if (!resource)
            return {};

        if (inStream.getState() != EStatus::Ok)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "ResourcePersistation::RetrieveResourceFromStream: resource deserialization failed for {}", fileEntry.resourceInfo.hash);
            return {};
        }

        UInt currentPosAfterRead = 0;
        const EStatus posStatus = inStream.getPos(currentPosAfterRead);
        if (posStatus != EStatus::Ok || currentPosAfterRead - fileEntry.offsetInBytes != fileEntry.sizeInBytes)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "ResourcePersistation::RetrieveResourceFromStream: read position fail for {}, state {} (resOffset {}, resSize {}, filePos{})",
                        fileEntry.resourceInfo.hash, posStatus, fileEntry.offsetInBytes, fileEntry.sizeInBytes, currentPosAfterRead);
            return {};
        }

        return resource;
    }
}
