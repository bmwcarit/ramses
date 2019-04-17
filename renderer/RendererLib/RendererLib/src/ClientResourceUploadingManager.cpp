//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ClientResourceUploadingManager.h"
#include "RendererLib/RendererClientResourceRegistry.h"
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
    ClientResourceUploadingManager::ClientResourceUploadingManager(
        RendererClientResourceRegistry& resources,
        IResourceUploader& uploader,
        IRenderBackend& renderBackend,
        Bool keepEffects,
        const FrameTimer& frameTimer,
        RendererStatistics& stats,
        UInt64 clientResourceCacheSize)
        : m_clientResources(resources)
        , m_uploader(uploader)
        , m_renderBackend(renderBackend)
        , m_keepEffects(keepEffects)
        , m_frameTimer(frameTimer)
        , m_clientResourceCacheSize(clientResourceCacheSize)
        , m_stats(stats)
    {
    }

    ClientResourceUploadingManager::~ClientResourceUploadingManager()
    {
        // Unload all remaining resources that were kept due to caching strategy.
        // Or in case display is being destructed together with scenes and there is no more rendering,
        // ie. no more deferred upload/unloads
        ResourceContentHashVector resourcesToUnload;
        getClientResourcesToUnloadNext(resourcesToUnload, false, std::numeric_limits<UInt64>::max());
        unloadClientResources(resourcesToUnload);

        for(const auto& resource : m_clientResources.getAllResourceDescriptors())
        {
            UNUSED(resource);
            assert(resource.value.status != EResourceStatus_Uploaded);
        }
    }

    Bool ClientResourceUploadingManager::hasAnythingToUpload() const
    {
        return !m_clientResources.getAllProvidedResources().empty();
    }

    void ClientResourceUploadingManager::uploadAndUnloadPendingResources()
    {
        ResourceContentHashVector resourcesToUpload;
        UInt64 sizeToUpload = 0u;
        getAndPrepareClientResourcesToUploadNext(resourcesToUpload, sizeToUpload);
        const UInt64 sizeToBeFreed = getAmountOfMemoryToBeFreedForNewResources(sizeToUpload);

        ResourceContentHashVector resourcesToUnload;
        getClientResourcesToUnloadNext(resourcesToUnload, m_keepEffects, sizeToBeFreed);

        unloadClientResources(resourcesToUnload);
        uploadClientResources(resourcesToUpload);
    }

    void ClientResourceUploadingManager::unloadClientResources(const ResourceContentHashVector& resourcesToUnload)
    {
        for(const auto& resource : resourcesToUnload)
        {
            const ResourceDescriptor& rd = m_clientResources.getResourceDescriptor(resource);
            unloadClientResource(rd);
        }
    }

    void ClientResourceUploadingManager::uploadClientResources(const ResourceContentHashVector& resourcesToUpload)
    {
        UInt32 sizeUploaded = 0u;
        for (UInt32 i = 0; i < resourcesToUpload.size(); ++i)
        {
            const ResourceDescriptor& rd = m_clientResources.getResourceDescriptor(resourcesToUpload[i]);
            const UInt32 resourceSize = rd.resource.getResourceObject()->getDecompressedDataSize();
            uploadClientResource(rd);
            m_stats.clientResourceUploaded(resourceSize);
            sizeUploaded += resourceSize;

            const Bool checkTimeLimit = (i % NumResourcesToUploadInBetweenTimeBudgetChecks == 0) || rd.type == EResourceType_Effect || resourceSize > LargeResourceByteSizeThreshold;
            if (checkTimeLimit && m_frameTimer.isTimeBudgetExceededForSection(EFrameTimerSectionBudget::ClientResourcesUpload))
            {
                const auto numUploaded = i + 1;
                const auto numRemaining = resourcesToUpload.size() - numUploaded;
                LOG_INFO(CONTEXT_RENDERER, "ClientResourceUploadingManager::uploadClientResources: Interrupt: Exceeded time for client resource upload (uploaded " << numUploaded << " resources of size " << sizeUploaded << " B, remaining " << numRemaining << " resources to upload)");
                LOG_INFO_F(CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& logger)
                {
                    logger << "Remaining resources in queue to upload:";
                    for (UInt32 j = numUploaded; j < resourcesToUpload.size() && j < numUploaded + 10; ++j)
                    {
                        const ResourceDescriptor& interruptedRd = m_clientResources.getResourceDescriptor(resourcesToUpload[j]);
                        logger << " [" << interruptedRd.hash << "; " << EnumToString(interruptedRd.type) << "; " << interruptedRd.decompressedSize << " B]";
                    }
                    if (numRemaining > 10)
                        logger << " ...";
                });

                break;
            }
        }
    }

    void ClientResourceUploadingManager::uploadClientResource(const ResourceDescriptor& rd)
    {
        assert(rd.resource.getResourceObject() != NULL);
        assert(!rd.deviceHandle.isValid());
        LOG_TRACE(CONTEXT_PROFILING, "        ResourceUploadingManager::uploadResource upload resource of type " << EnumToString(rd.type));

        const IResource* pResource = rd.resource.getResourceObject();
        assert(pResource->isDeCompressedAvailable());

        const UInt32 resourceSize = pResource->getDecompressedDataSize();
        UInt32 vramSize = 0;
        const DeviceResourceHandle deviceHandle = m_uploader.uploadResource(m_renderBackend, rd.resource, vramSize);
        if (deviceHandle.isValid())
        {
            m_clientResourceSizes.put(rd.hash, resourceSize);
            m_clientResourceTotalUploadedSize += resourceSize;
            m_clientResources.setResourceStatus(rd.hash, EResourceStatus_Uploaded);
            m_clientResources.setResourceSize(rd.hash, pResource->getCompressedDataSize(), resourceSize, vramSize);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "ResourceUploadingManager::uploadResource failed to upload resource #" << StringUtils::HexFromResourceContentHash(rd.hash) << " (" << EnumToString(rd.type) << ")");
            m_clientResources.setResourceStatus(rd.hash, EResourceStatus_Broken);
        }

        // release reference to managed resource
        m_clientResources.setResourceData(rd.hash, ManagedResource(), deviceHandle, pResource->getTypeID());
    }

    void ClientResourceUploadingManager::unloadClientResource(const ResourceDescriptor& rd)
    {
        assert(rd.sceneUsage.empty());
        assert(rd.status == EResourceStatus_Uploaded);
        assert(m_clientResourceSizes.contains(rd.hash));

        LOG_TRACE(CONTEXT_PROFILING, "        ResourceUploadingManager::unloadResource delete resource of type " << EnumToString(rd.type));
        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploadingManager::unloadResource Unloading resource #" << rd.hash);
        m_uploader.unloadResource(m_renderBackend, rd.type, rd.hash, rd.deviceHandle);

        auto resSizeIt = m_clientResourceSizes.find(rd.hash);
        assert(m_clientResourceTotalUploadedSize >= resSizeIt->value);
        m_clientResourceTotalUploadedSize -= resSizeIt->value;
        m_clientResourceSizes.remove(resSizeIt);

        LOG_TRACE(CONTEXT_RENDERER, "ResourceUploadingManager::unloadResource Removing resource descriptor for resource #" << rd.hash);
        m_clientResources.unregisterResource(rd.hash);
    }

    void ClientResourceUploadingManager::getClientResourcesToUnloadNext(ResourceContentHashVector& resourcesToUnload, Bool keepEffects, UInt64 sizeToBeFreed) const
    {
        assert(resourcesToUnload.empty());
        const ResourceContentHashVector& unusedResources = m_clientResources.getAllResourcesNotInUseByScenes();
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

            const ResourceDescriptor& rd = m_clientResources.getResourceDescriptor(hash);
            if (rd.status == EResourceStatus_Uploaded)
            {
                const Bool keepEffectCached = keepEffects && (rd.type == EResourceType_Effect);
                if (!keepEffectCached)
                {
                    resourcesToUnload.push_back(hash);
                    assert(m_clientResourceSizes.contains(hash));
                    sizeToUnload += *m_clientResourceSizes.get(hash);
                }
            }
        }
    }

    void ClientResourceUploadingManager::getAndPrepareClientResourcesToUploadNext(ResourceContentHashVector& resourcesToUpload, UInt64& totalSize) const
    {
        assert(resourcesToUpload.empty());

        totalSize = 0u;
        const ResourceContentHashVector& providedResources = m_clientResources.getAllProvidedResources();
        for(const auto& resource : providedResources)
        {
            const ResourceDescriptor& rd = m_clientResources.getResourceDescriptor(resource);
            assert(rd.status == EResourceStatus_Provided);
            assert(rd.resource.getResourceObject() != NULL);
            const IResource* resourceObj = rd.resource.getResourceObject();
            resourceObj->decompress();
            totalSize += resourceObj->getDecompressedDataSize();

            resourcesToUpload.push_back(resource);
        }
    }

    UInt64 ClientResourceUploadingManager::getAmountOfMemoryToBeFreedForNewResources(UInt64 sizeToUpload) const
    {
        if (m_clientResourceCacheSize == 0u)
        {
            // unload all if no caching is allowed
            return std::numeric_limits<UInt64>::max();
        }

        if (m_clientResourceCacheSize > m_clientResourceTotalUploadedSize)
        {
            const UInt64 remainingCacheSize = m_clientResourceCacheSize - m_clientResourceTotalUploadedSize;
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
            return sizeToUpload + m_clientResourceTotalUploadedSize - m_clientResourceCacheSize;
        }
    }
}
