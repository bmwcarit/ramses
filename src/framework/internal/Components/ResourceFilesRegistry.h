//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ResourceStorage.h"
#include "ManagedResource.h"
#include "internal/Components/ResourceHashUsage.h"
#include "ResourceTableOfContents.h"
#include "internal/Components/InputStreamContainer.h"
#include "internal/Core/Utils/File.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/Components/SceneFileHandle.h"

#include <algorithm>
#include <unordered_map>

namespace ramses::internal
{
    struct ResourceRegistryEntry
    {
        ResourceFileEntry fileEntry;
        ResourceHashUsage hashUsage;

        ResourceRegistryEntry(const ResourceFileEntry& fileEntry_, ResourceHashUsage hashUsage_)
            : fileEntry(fileEntry_)
            , hashUsage(std::move(hashUsage_))
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
        [[nodiscard]] const FileContentsMap* getContentsOfResourceFile(SceneFileHandle handle) const;
        EStatus getEntry(const ResourceContentHash& hash, IInputStream*& resourceStream, ResourceFileEntry& fileEntry, SceneFileHandle& fileHandle) const;
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
    EStatus ResourceFilesRegistry::getEntry(const ResourceContentHash& hash, IInputStream*& resourceStream, ResourceFileEntry& fileEntry, SceneFileHandle& fileHandle) const
    {
        for (auto& iter : m_resourceFiles)
        {
            const FileContentsMap& fileContents = iter.second.resources;
            ResourceRegistryEntry* entry = fileContents.get(hash);
            if (entry != nullptr)
            {
                resourceStream = &iter.second.stream->getStream();
                fileEntry = entry->fileEntry;
                fileHandle = iter.first;
                return EStatus::Ok;
            }
        }
        return EStatus::NotExist;
    }
}
