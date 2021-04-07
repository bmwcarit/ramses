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
#include "Components/InputStreamContainer.h"
#include "Utils/File.h"
#include "Collections/Pair.h"
#include "Components/SceneFileHandle.h"

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

    using FileContentsMap = HashMap<ResourceContentHash, ResourceRegistryEntry>;

    struct ResourceRegistryFileEntry
    {
        InputStreamContainerSPtr stream;
        FileContentsMap resources;
    };

    class ResourceFilesRegistry
    {
    public:
        SceneFileHandle registerResourceFile(const InputStreamContainerSPtr& resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage);
        void unregisterResourceFile(SceneFileHandle handle);
        const FileContentsMap* getContentsOfResourceFile(SceneFileHandle handle) const;
        EStatus getEntry(const ResourceContentHash& hash, IInputStream*& resourceStream, ResourceFileEntry& fileEntry) const;
    private:
        std::unordered_map<SceneFileHandle, ResourceRegistryFileEntry> m_resourceFiles;
        SceneFileHandle m_nextHandle{1};
    };

    inline
    SceneFileHandle ResourceFilesRegistry::registerResourceFile(const InputStreamContainerSPtr& resourceFileInputStream, const ResourceTableOfContents& toc, ResourceStorage& resourceStorage)
    {
        assert(resourceFileInputStream);

        const auto& tocContent = toc.getFileContents();
        FileContentsMap fileContentsMap(tocContent.size());
        for (const auto& p : tocContent)
            fileContentsMap.put(p.key, ResourceRegistryEntry{p.value, resourceStorage.getResourceHashUsage(p.key)});

        SceneFileHandle handle = m_nextHandle;
        m_resourceFiles.insert({ handle, ResourceRegistryFileEntry{resourceFileInputStream, std::move(fileContentsMap)} });
        ++m_nextHandle.getReference();
        return handle;
    }

    inline
    void ResourceFilesRegistry::unregisterResourceFile(SceneFileHandle handle)
    {
        m_resourceFiles.erase(handle);
    }

    inline
    const FileContentsMap* ResourceFilesRegistry::getContentsOfResourceFile(SceneFileHandle handle) const
    {
        const auto it = m_resourceFiles.find(handle);
        if (it == m_resourceFiles.end())
            return nullptr;
        return &it->second.resources;
    }

    inline
    EStatus ResourceFilesRegistry::getEntry(const ResourceContentHash& hash, IInputStream*& resourceStream, ResourceFileEntry& fileEntry) const
    {
        for (auto& iter : m_resourceFiles)
        {
            const FileContentsMap& fileContents = iter.second.resources;
            ResourceRegistryEntry* entry = fileContents.get(hash);
            if (entry != nullptr)
            {
                resourceStream = &iter.second.stream->getStream();
                fileEntry = entry->fileEntry;
                return EStatus::Ok;
            }
        }
        return EStatus::NotExist;
    }
}

#endif
