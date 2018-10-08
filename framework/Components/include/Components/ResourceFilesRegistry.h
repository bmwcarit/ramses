//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILESREGISTRY_H
#define RAMSES_RESOURCEFILESREGISTRY_H

#include "ResourceStorage.h"
#include "ManagedResource.h"
#include "Components/ResourceHashUsage.h"
#include "ResourceTableOfContents.h"
#include "ResourceFileInputStream.h"
#include "Utils/File.h"
#include "Collections/Pair.h"

namespace ramses_internal
{
    struct ResourceRegistryEntry
    {
        ResourceFileEntry fileEntry;
        ResourceHashUsage hashUsage;

        ResourceRegistryEntry(const ResourceFileEntry& fileEntry_, const ResourceHashUsage& hashUsage_)
            : fileEntry(fileEntry_)
            , hashUsage(hashUsage_)
        {}

        ResourceRegistryEntry()
        {}
    };
    typedef HashMap<ResourceContentHash, ResourceRegistryEntry> FileContentsMap;
    typedef HashMap<ResourceFileInputStreamSPtr, FileContentsMap> ResourceFileInputStreamToFileContentMap;

    class ResourceFilesRegistry
    {
    public:
        void registerResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage);
        void unregisterResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream);
        void unregisterResourceFile(const String& filename);
        bool hasResourceFile(const String& resourceFileName) const;

        bool canLoadResource(const ResourceContentHash& hash) const;
        EStatus getEntry(const ResourceContentHash& hash, BinaryFileInputStream*& resourceStream, ResourceFileEntry& fileEntry) const;
    private:
        ResourceFileInputStreamToFileContentMap m_resourceFiles;
    };

    inline
    void ResourceFilesRegistry::registerResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage)
    {
        assert(resourceFileInputStream);

        FileContentsMap fileContentsMap;
        for (const auto& iter : toc.getFileContents())
        {
            ResourceRegistryEntry entry(iter.value, resourceStorage.getResourceHashUsage(iter.key));
            fileContentsMap.put(iter.key, entry);
        }
        m_resourceFiles.put(resourceFileInputStream, fileContentsMap);
    }

    inline
    void ResourceFilesRegistry::unregisterResourceFile(const String& filename)
    {
        for (auto iter = m_resourceFiles.begin(); iter != m_resourceFiles.end(); iter++)
        {
            if (iter->key->getResourceFileName() == filename)
            {
                m_resourceFiles.remove(iter);
                break;
            }
        }
    }

    inline
    void ResourceFilesRegistry::unregisterResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream)
    {
        m_resourceFiles.remove(resourceFileInputStream);
    }

    inline
    bool ResourceFilesRegistry::hasResourceFile(const String& resourceFileName) const
    {
        for (const auto& iter : m_resourceFiles)
        {
            if (iter.key->getResourceFileName() == resourceFileName)
            {
                return true;
            }
        }
        return false;
    }

    inline
    EStatus ResourceFilesRegistry::getEntry(const ResourceContentHash& hash, BinaryFileInputStream*& resourceStream, ResourceFileEntry& fileEntry) const
    {
        for (const auto& iter : m_resourceFiles)
        {
            const FileContentsMap& fileContents = iter.value;
            ResourceRegistryEntry* entry = fileContents.get(hash);
            if (entry != 0)
            {
                resourceStream = &iter.key->resourceStream;
                fileEntry = entry->fileEntry;
                return EStatus_RAMSES_OK;
            }
        }
        return EStatus_RAMSES_NOT_EXIST;
    }

}

#endif
