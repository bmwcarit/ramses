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

#include <algorithm>
#include <unordered_map>

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
    };
    typedef HashMap<ResourceContentHash, ResourceRegistryEntry> FileContentsMap;
    typedef std::unordered_map<ResourceFileInputStreamSPtr, FileContentsMap> ResourceFileInputStreamToFileContentMap;

    class ResourceFilesRegistry
    {
    public:
        void registerResourceFile(const ResourceFileInputStreamSPtr& resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage);
        void unregisterResourceFile(const ResourceFileInputStreamSPtr& resourceFileInputStream);
        void unregisterResourceFile(const String& filename);
        bool hasResourceFile(const String& resourceFileName) const;
        const FileContentsMap* getContentsOfResourceFile(const String& filename) const;
        EStatus getEntry(const ResourceContentHash& hash, BinaryFileInputStream*& resourceStream, ResourceFileEntry& fileEntry) const;
    private:
        ResourceFileInputStreamToFileContentMap m_resourceFiles;
    };

    inline
    void ResourceFilesRegistry::registerResourceFile(const ResourceFileInputStreamSPtr& resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage)
    {
        assert(resourceFileInputStream);

        FileContentsMap fileContentsMap;
        for (const auto& iter : toc.getFileContents())
        {
            ResourceRegistryEntry entry(iter.value, resourceStorage.getResourceHashUsage(iter.key));
            fileContentsMap.put(iter.key, entry);
        }
        m_resourceFiles.insert({ resourceFileInputStream, fileContentsMap });
    }

    inline
    void ResourceFilesRegistry::unregisterResourceFile(const String& filename)
    {
        for (auto iter = m_resourceFiles.begin(); iter != m_resourceFiles.end(); iter++)
        {
            if (iter->first->getResourceFileName() == filename)
            {
                m_resourceFiles.erase(iter);
                break;
            }
        }
    }

    inline
    const FileContentsMap* ResourceFilesRegistry::getContentsOfResourceFile(const String& filename) const
    {
        auto it = std::find_if(m_resourceFiles.begin(), m_resourceFiles.end(), [&filename](auto const& entry) {
            return entry.first->getResourceFileName() == filename;
        });
        return it == m_resourceFiles.end() ? nullptr : &it->second;
    }

    inline
    void ResourceFilesRegistry::unregisterResourceFile(const ResourceFileInputStreamSPtr& resourceFileInputStream)
    {
        m_resourceFiles.erase(resourceFileInputStream);
    }

    inline
    bool ResourceFilesRegistry::hasResourceFile(const String& resourceFileName) const
    {
        for (const auto& iter : m_resourceFiles)
        {
            if (iter.first->getResourceFileName() == resourceFileName)
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
            const FileContentsMap& fileContents = iter.second;
            ResourceRegistryEntry* entry = fileContents.get(hash);
            if (entry != nullptr)
            {
                resourceStream = &iter.first->resourceStream;
                fileEntry = entry->fileEntry;
                return EStatus::Ok;
            }
        }
        return EStatus::NotExist;
    }

}

#endif
