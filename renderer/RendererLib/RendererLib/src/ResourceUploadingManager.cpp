//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ResourceUploadingManager.h"
#include "RendererLib/RendererResourceRegistry.h"
#include "RendererLib/IResourceUploader.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/IDevice.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    ResourceUploadingManager::ResourceUploadingManager(
        RendererResourceRegistry& resources,
        IResourceUploader& uploader,
        IRenderBackend& renderBackend,
        Bool keepEffects,
        const FrameTimer& frameTimer,
        RendererStatistics& stats,
        UInt64 gpuCacheSize)
        : m_resources(resources)
        , m_uploader(uploader)
        , m_renderBackend(renderBackend)
        , m_keepEffects(keepEffects)
        , m_frameTimer(frameTimer)
        , m_resourceCacheSize(gpuCacheSize)
        , m_stats(stats)
    {
    }

    ResourceUploadingManager::~ResourceUploadingManager()
    {
        // Unload all remaining resources that were kept due to caching strategy.
        // Or in case display is being destructed together with scenes and there is no more rendering,
        // ie. no more deferred upload/unloads
        ResourceContentHashVector resourcesToUnload;
        getResourcesToUnloadNext(resourcesToUnload, false, std::numeric_limits<UInt64>::max());
        unloadResources(resourcesToUnload);

        for(const auto& resource : m_resources.getAllResourceDescriptors())
        {
            UNUSED(resource);
            assert(resource.value.status != EResourceStatus::Uploaded);
        }
    }

    Bool ResourceUploadingManager::hasAnythingToUpload() const
    {
        return !m_resources.getAllProvidedResources().empty();
    }

    void ResourceUploadingManager::uploadAndUnloadPendingResources()
    {
        ResourceContentHashVector resourcesToUpload;
        UInt64 sizeToUpload = 0u;
        getAndPrepareResourcesToUploadNext(resourcesToUpload, sizeToUpload);
        const UInt64 sizeToBeFreed = getAmountOfMemoryToBeFreedForNewResources(sizeToUpload);

        ResourceContentHashVector resourcesToUnload;
        getResourcesToUnloadNext(resourcesToUnload, m_keepEffects, sizeToBeFreed);

        unloadResources(resourcesToUnload);
        uploadResources(resourcesToUpload);
    }

    void ResourceUploadingManager::unloadResources(const ResourceContentHashVector& resourcesToUnload)
    {
        for(const auto& resource : resourcesToUnload)
        {
            const ResourceDescriptor& rd = m_resources.getResourceDescriptor(resource);
            unloadResource(rd);
        }
    }

    void ResourceUploadingManager::uploadResources(const ResourceContentHashVector& resourcesToUpload)
    {
        UInt32 sizeUploaded = 0u;
        for (size_t i = 0; i < resourcesToUpload.size(); ++i)
        {
            const ResourceDescriptor& rd = m_resources.getResourceDescriptor(resourcesToUpload[i]);
            const UInt32 resourceSize = rd.resource->getDecompressedDataSize();
            uploadResource(rd);
            m_stats.resourceUploaded(resourceSize);
            sizeUploaded += resourceSize;

            const Bool checkTimeLimit = (i % NumResourcesToUploadInBetweenTimeBudgetChecks == 0) || rd.type == EResourceType_Effect || resourceSize > LargeResourceByteSizeThreshold;
            if (checkTimeLimit && m_frameTimer.isTimeBudgetExceededForSection(EFrameTimerSectionBudget::ResourcesUpload))
            {
                const auto numUploaded = i + 1;
                const auto numRemaining = resourcesToUpload.size() - numUploaded;
                LOG_INFO(CONTEXT_RENDERER, "ResourceUploadingManager::uploadResources: Interrupt: Exceeded time for resource upload (uploaded " << numUploaded << " resources of size " << sizeUploaded << " B, remaining " << numRemaining << " resources to upload)");
                LOG_INFO_F(CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& logger)
                {
                    logger << "Remaining resources in queue to upload:";
                    for (size_t j = numUploaded; j < resourcesToUpload.size() && j < numUploaded + 10; ++j)
                    {
                        const ResourceDescriptor& interruptedRd = m_resources.getResourceDescriptor(resourcesToUpload[j]);
                        logger << " [" << interruptedRd.hash << "; " << EnumToString(interruptedRd.type) << "]";
                    }
                    if (numRemaining > 10)
                        logger << " ...";
                });

                break;
            }
        }
    }

    void ResourceUploadingManager::uploadResource(const ResourceDescriptor& rd)
    {
        assert(rd.resource);
        assert(!rd.deviceHandle.isValid());
        LOG_TRACE(CONTEXT_PROFILING, "        ResourceUploadingManager::uploadResource upload resource of type " << EnumToString(rd.type));

        const IResource* pResource = rd.resource.get();
        assert(pResource->isDeCompressedAvailable());

        const UInt32 resourceSize = pResource->getDecompressedDataSize();
        UInt32 vramSize = 0;
        const DeviceResourceHandle deviceHandle = m_uploader.uploadResource(m_renderBackend, rd, vramSize);
        if (deviceHandle.isValid())
        {
            m_resourceSizes.put(rd.hash, resourceSize);
            m_resourceTotalUploadedSize += resourceSize;
            m_resources.setResourceUploaded(rd.hash, deviceHandle, vramSize);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "ResourceUploadingManager::uploadResource failed to upload resource #" << rd.hash << " (" << EnumToString(rd.type) << ")");
            m_resources.setResourceBroken(rd.hash);
        }
    }

    void ResourceUploadingManager::unloadResource(const ResourceDescriptor& rd)
    {
        assert(rd.sceneUsage.empty());
        assert(rd.status == EResourceStatus::Uploaded);
        assert(m_resourceSizes.contains(rd.hash));

        LOG_TRACE(CONTEXT_PROFILING, "        ResourceUploadingManager::unloadResource delete resource of type " << EnumToString(rd.type));
        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploadingManager::unloadResource Unloading resource #" << rd.hash);
        m_uploader.unloadResource(m_renderBackend, rd.type, rd.hash, rd.deviceHandle);

        auto resSizeIt = m_resourceSizes.find(rd.hash);
        assert(m_resourceTotalUploadedSize >= resSizeIt->value);
        m_resourceTotalUploadedSize -= resSizeIt->value;
        m_resourceSizes.remove(resSizeIt);

        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploadingManager::unloadResource Removing resource descriptor for resource #" << rd.hash);
        m_resources.unregisterResource(rd.hash);
    }

    void ResourceUploadingManager::getResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, Bool keepEffects, UInt64 sizeToBeFreed) const
    {
        assert(resourcesToUnload.empty());
        const ResourceContentHashVector& unusedResources = m_resources.getAllResourcesNotInUseByScenes();
        UInt64 sizeToUnload = 0u;

        // collect unused resources to be unloaded
        // if total size of resources to be unloaded is enough
        // we stop adding more unused resources, they can be kept uploaded as long as not more memory is needed
        for (const auto& hash : unusedResources)
        {
            if (sizeToUnload >= sizeToBeFreed)
            {
                break;
            }

            const ResourceDescriptor& rd = m_resources.getResourceDescriptor(hash);
            if (rd.status == EResourceStatus::Uploaded)
            {
                const Bool keepEffectCached = keepEffects && (rd.type == EResourceType_Effect);
                if (!keepEffectCached)
                {
                    resourcesToUnload.push_back(hash);
                    assert(m_resourceSizes.contains(hash));
                    sizeToUnload += *m_resourceSizes.get(hash);
                }
            }
        }
    }

    void ResourceUploadingManager::getAndPrepareResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, UInt64& totalSize) const
    {
        assert(resourcesToUpload.empty());

        totalSize = 0u;
        const ResourceContentHashVector& providedResources = m_resources.getAllProvidedResources();
        for(const auto& resource : providedResources)
        {
            const ResourceDescriptor& rd = m_resources.getResourceDescriptor(resource);
            assert(rd.status == EResourceStatus::Provided);
            assert(rd.resource);
            const IResource* resourceObj = rd.resource.get();
            resourceObj->decompress();
            totalSize += resourceObj->getDecompressedDataSize();

            resourcesToUpload.push_back(resource);
        }
    }

    UInt64 ResourceUploadingManager::getAmountOfMemoryToBeFreedForNewResources(UInt64 sizeToUpload) const
    {
        if (m_resourceCacheSize == 0u)
        {
            // unload all if no caching is allowed
            return std::numeric_limits<UInt64>::max();
        }

        if (m_resourceCacheSize > m_resourceTotalUploadedSize)
        {
            const UInt64 remainingCacheSize = m_resourceCacheSize - m_resourceTotalUploadedSize;
            if (remainingCacheSize < sizeToUpload)
            {
                return sizeToUpload - remainingCacheSize;
            }
            else
            {
                return 0u;
            }
        }
        else
        {
            // cache already exceeded, try unloading all that is above cache limit plus size for new resources to be uploaded
            return sizeToUpload + m_resourceTotalUploadedSize - m_resourceCacheSize;
        }
    }
}
