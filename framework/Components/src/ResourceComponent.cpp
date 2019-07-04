//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceComponent.h"
#include "Transfer/ResourceTypes.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/ResourceFilesRegistry.h"
#include "TaskFramework/ITaskQueue.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/StringUtils.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryInputStream.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "Components/ResourceStreamSerialization.h"
#include <algorithm>
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    ResourceComponent::ResourceComponent(ITaskQueue& queue, const Guid& myAddress, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
        StatisticCollectionFramework& statistics, PlatformLock& frameworkLock, uint32_t maximumTotalBytesForAsynResourceLoading)
        : m_frameworkLock(frameworkLock)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_resourceStorage(frameworkLock)
        , m_taskQueueForResourceLoading(queue)
        , m_maximumBytesAllowedForResourceLoading(maximumTotalBytesForAsynResourceLoading)
        , m_bytesScheduledForLoading(0)
        , m_communicationSystem(communicationSystem)
        , m_myAddress(myAddress)
        , m_statistics(statistics)
    {
        m_connectionStatusUpdateNotifier.registerForConnectionUpdates(this);

        m_resourceStorage.setListener(*this);
        m_communicationSystem.setResourceConsumerServiceHandler(this);
        m_communicationSystem.setResourceProviderServiceHandler(this);
    }

    ResourceComponent::~ResourceComponent()
    {
        m_communicationSystem.setResourceConsumerServiceHandler(NULL);
        m_communicationSystem.setResourceProviderServiceHandler(NULL);

        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);
        m_taskQueueForResourceLoading.disableAcceptingTasksAfterExecutingCurrentQueue();

        for (auto& p : m_resourceDeserializers)
        {
            delete p.value;
        }
    }

    ramses_internal::ManagedResource ResourceComponent::getResource(ResourceContentHash hash)
    {
        return m_resourceStorage.getResource(hash);
    }

    ramses_internal::ResourceHashUsage ResourceComponent::getResourceHashUsage(const ResourceContentHash& hash)
    {
        return m_resourceStorage.getResourceHashUsage(hash);
    }

    ManagedResourceVector ResourceComponent::getResources()
    {
        return m_resourceStorage.getResources();
    }

    ramses_internal::ManagedResource ResourceComponent::manageResource(const IResource& resource, Bool deletionAllowed)
    {
        return m_resourceStorage.manageResource(resource, deletionAllowed);
    }

    void ResourceComponent::storeResourceInfo(const ResourceContentHash& hash, const ResourceInfo& resourceInfo)
    {
        m_resourceStorage.storeResourceInfo(hash, resourceInfo);
    }

    const ResourceInfo& ResourceComponent::getResourceInfo(const ResourceContentHash& hash) const
    {
        return m_resourceStorage.getResourceInfo(hash);
    }

    void ResourceComponent::handleRequestResources(const ResourceContentHashVector& ids, UInt32 chunkSize, const Guid& requesterId)
    {
        UNUSED(chunkSize);

        ManagedResourceVector resourceToSendViaNetwork;
        ResourceContentHashVector unavailableResources;
        ResourceContentHashVector resourcesToBeLoaded;

        for(const auto& id : ids)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: " << StringUtils::HexFromResourceContentHash(id) << " from " << requesterId);
            const ManagedResource& resource = m_resourceStorage.getResource(id);
            if (resource.getResourceObject())
            {
                LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: sendResource" << StringUtils::HexFromResourceContentHash(id) << " name: " << resource.getResourceObject()->getName());
                resourceToSendViaNetwork.push_back(resource);
            }
            else
            {
                // try resource files
                ResourceLoadInfo loadInfo;
                const EStatus canLoadFromFile = m_resourceFiles.getEntry(id, loadInfo.resourceStream, loadInfo.fileEntry);
                if (canLoadFromFile == EStatus_RAMSES_OK)
                {
                    loadInfo.requesterId = requesterId;
                    m_resourcesToBeLoaded.push_back(loadInfo);
                    resourcesToBeLoaded.push_back(id);
                }
                else
                {
                    unavailableResources.push_back(id);
                    LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: Could not find or load requested resource: " << StringUtils::HexFromResourceContentHash(id) << " requested by " << requesterId);
                }
            }
        }

        if (!resourceToSendViaNetwork.empty())
        {
            m_communicationSystem.sendResources(requesterId, resourceToSendViaNetwork);
        }

        if (unavailableResources.size() > 0)
        {
            m_communicationSystem.sendResourcesNotAvailable(requesterId, unavailableResources);
        }

        LOG_INFO_F(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "ResourceComponent::handleRequestResources(from " << requesterId << "): ";

            if (!resourceToSendViaNetwork.empty())
            {
                sos << "send " << resourceToSendViaNetwork.size() << " locally available ";
                for (const auto& res : resourceToSendViaNetwork)
                    sos << StringUtils::HexFromResourceContentHash(res.getResourceObject()->getHash()) << " ";
                sos << "; ";
            }
            if (!resourcesToBeLoaded.empty())
            {
                sos << "load " << resourcesToBeLoaded.size() << " from file ";
                for (const auto& hash : resourcesToBeLoaded)
                    sos << StringUtils::HexFromResourceContentHash(hash) << " ";
                sos << "; ";
            }
            if (!unavailableResources.empty())
            {
                sos << "send " << unavailableResources.size() << " unavailable ";
                for (const auto& hash : unavailableResources)
                    sos << StringUtils::HexFromResourceContentHash(hash) << " ";
            }
        }));

        triggerLoadingResourcesFromFile();
    }

    void ResourceComponent::requestResourceAsynchronouslyFromFramework(const ResourceContentHashVector& resourceHashes, const RequesterID& requesterID, const Guid& providerID)
    {
        ResourceContentHashVector resourcesToBeRetrievedFromProvider;
        ResourceContentHashVector resourcesToBeLoaded;
        ResourceContentHashVector resourcesLocallyAvailable;

        for (const auto hash : resourceHashes)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::requestResourceAsynchronouslyFromFramework:" << StringUtils::HexFromResourceContentHash(hash));
            ManagedResource managedResource = m_resourceStorage.getResource(hash);

            if (managedResource.getResourceObject() != 0)
            {
                // already available
                resourcesLocallyAvailable.push_back(hash);
                m_arrivedResources[requesterID].push_back(managedResource);
            }
            else
            {
                const auto range = m_requestedResources.equal_range(hash);
                const bool mustRequestResource = range.first == range.second;

                if (FindInRequestsRange(range, requesterID) == range.second)
                {
                    // add to requests if not yet requested by this requester
                    m_requestedResources.insert(std::make_pair(hash, requesterID));
                }

                if (mustRequestResource)
                {
                    // only trigger request from file or network if not already requested by any requester
                    ResourceLoadInfo loadInfo;
                    const EStatus canLoadResource = m_resourceFiles.getEntry(hash, loadInfo.resourceStream, loadInfo.fileEntry);
                    if (canLoadResource == EStatus_RAMSES_OK)
                    {
                        m_resourcesToBeLoaded.push_back(loadInfo);
                        LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::requestResourceAsynchronouslyFromFramework: resource found in resource file, will be loaded later:" << StringUtils::HexFromResourceContentHash(hash));
                    }
                    else
                    {
                        // only call communication system for non-local providers
                        if (m_myAddress != providerID)
                        {
                            resourcesToBeRetrievedFromProvider.push_back(hash);
                        }
                        else
                        {
                            // error, want to ask myself, which means it is not available right now
                            LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceComponent::requestResourceAsynchronouslyFromFramework: Resource not available locally: " << hash);
                        }
                    }
                }
            }
        }

        if (!resourcesToBeRetrievedFromProvider.empty())
        {
            LOG_INFO_F(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
                sos << "ResourceComponent::requestResourceAsynchronouslyFromFramework(provider " << providerID << ", requester " << requesterID
                    << "): " << "request from network " << resourcesToBeRetrievedFromProvider.size() << " resources: ";
                for (const auto& hash : resourcesToBeRetrievedFromProvider)
                    sos << StringUtils::HexFromResourceContentHash(hash) << " ";
                sos << "; ";
            }));
            m_communicationSystem.sendRequestResources(providerID, resourcesToBeRetrievedFromProvider);
        }

        LOG_TRACE_F(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "ResourceComponent::requestResourceAsynchronouslyFromFramework(provider " << providerID << ", requester " << requesterID << "): ";

            if (!resourcesLocallyAvailable.empty())
            {
                sos << "locally available " << resourcesLocallyAvailable.size() << " resources: ";
                for (const auto& hash : resourcesLocallyAvailable)
                    sos << StringUtils::HexFromResourceContentHash(hash) << " ";
                sos << "; ";
            }
            if (!resourcesToBeLoaded.empty())
            {
                sos << "load from file " << resourcesToBeLoaded.size() << " resources: ";
                for (const auto& hash : resourcesToBeLoaded)
                    sos << StringUtils::HexFromResourceContentHash(hash) << " ";
                sos << "; ";
            }
        }));

        triggerLoadingResourcesFromFile();
    }

    void ResourceComponent::onBytesNeededByStorageDecreased(uint64_t /*bytes*/)
    {
        triggerLoadingResourcesFromFile();
    }

    void ResourceComponent::triggerLoadingResourcesFromFile()
    {
        PlatformGuard guard(m_frameworkLock);
        std::vector<ResourceLoadInfo> toLoadNow;
        bool shouldReserve = true;
        while (m_resourcesToBeLoaded.size() > 0)
        {
            const ResourceLoadInfo nextResourceToBeLoaded = m_resourcesToBeLoaded[0];
            const uint64_t bytesUsedOrScheduled = m_resourceStorage.getBytesUsedByResourcesInMemory() + m_bytesScheduledForLoading;
            const UInt32 sizeOfNextResource = nextResourceToBeLoaded.fileEntry.sizeInBytes;
            if (bytesUsedOrScheduled + sizeOfNextResource < m_maximumBytesAllowedForResourceLoading)
            {
                if (shouldReserve)
                {
                    // reserve here to avoid allocations in the case that no resources to load or next one still not fitting
                    toLoadNow.reserve(m_resourcesToBeLoaded.size());
                    shouldReserve = false;
                }

                // fits to load
                m_bytesScheduledForLoading += sizeOfNextResource;
                toLoadNow.push_back(nextResourceToBeLoaded);
                m_resourcesToBeLoaded.pop_front();
            }
            else
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::triggerLoadingResourcesFromFile: Could not load resources from file due exceeded memory limit, see RamsesFrameworkConfig::setMaximumTotalBytesAllowedForAsyncResourceLoading()" <<
                         " (usedOrScheduled:" << bytesUsedOrScheduled << " + wantToLoad:" << sizeOfNextResource << " < max:" << m_maximumBytesAllowedForResourceLoading << ", scheduled " << m_bytesScheduledForLoading << ")");
                break;
            }
        }
        if (toLoadNow.size() > 0)
        {
            LoadResourcesFromFileTask* task = new LoadResourcesFromFileTask(*this, std::move(toLoadNow), PlatformTime::GetMillisecondsMonotonic());
            m_taskQueueForResourceLoading.enqueue(*task);
            task->release();
        }
    }

    void ResourceComponent::cancelResourceRequest(const ResourceContentHash& resourceHash, const RequesterID& requesterID)
    {
        // remove from arrived
        bool removedFromArrived = false;
        auto& arrivedForRequester = m_arrivedResources[requesterID];
        for (auto it = arrivedForRequester.begin(); it != arrivedForRequester.end(); ++it)
        {
            if (it->getResourceObject()->getHash() == resourceHash)
            {
                arrivedForRequester.erase(it);
                removedFromArrived = true;
                break;
            }
        }

        // remove from requested
        bool removedFromRequested = false;
        const auto range = m_requestedResources.equal_range(resourceHash);
        const auto requesterIt = FindInRequestsRange(range, requesterID);
        if (requesterIt != range.second)
        {
            m_requestedResources.erase(requesterIt);
            removedFromRequested = true;
        }

        // check if cancel was valid
        if (!removedFromArrived && !removedFromRequested)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::cancelResourceRequest: hash " << resourceHash << " requested by " << requesterID << " was not found in arrived or requested");
        }
        else
        {
            LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::cancelResourceRequest: hash " << resourceHash << " requested by "
                << requesterID << " fromArrived " << removedFromArrived << " fromRequested " << removedFromRequested);
        }
    }

    ManagedResourceVector ResourceComponent::popArrivedResources(const RequesterID& requesterID)
    {
        ManagedResourceVector res;
        m_arrivedResources[requesterID].swap(res);
        LOG_INFO(CONTEXT_FRAMEWORK, "ResourceComponent::popArrivedResources: " << res.size()
            << " resources have arrived for requester " << requesterID);
        return res;
    }

    void ResourceComponent::addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ramses_internal::ResourceTableOfContents& toc)
    {
        for (const auto& item : toc.getFileContents())
        {
            storeResourceInfo(item.key, item.value.resourceInfo);
        }
        return m_resourceFiles.registerResourceFile(resourceFileInputStream, toc, m_resourceStorage);
    }

    bool ResourceComponent::hasResourceFile(const String& resourceFileName) const
    {
        return m_resourceFiles.hasResourceFile(resourceFileName);
    }

    void ResourceComponent::removeResourceFile(const String& resourceFileName)
    {
        m_resourceFiles.unregisterResourceFile(resourceFileName);
    }

    void ResourceComponent::handleSendResource(const ByteArrayView& receivedResourceData, const Guid& providerID)
    {
        PlatformGuard guard(m_frameworkLock);

        if (!m_resourceDeserializers.contains(providerID))
        {
            return;
        }
        ResourceStreamDeserializer* deserializer = *m_resourceDeserializers.get(providerID);
        assert(deserializer != nullptr);

        const std::vector<IResource*> resources = deserializer->processData(receivedResourceData);
        for (const auto& res : resources)
        {
            handleArrivedResource(m_resourceStorage.manageResource(*res, false));
        }
    }

    void ResourceComponent::handleArrivedResource(const ManagedResource& resource)
    {
        const ResourceContentHash hash = resource.getResourceObject()->getHash();

        LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleArrivedResource: resource available " << hash);

        const auto range = m_requestedResources.equal_range(hash);
        if (range.first == range.second)
        {
            LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::handleArrivedResource: Received unrequested resource with hash " << StringUtils::HexFromResourceContentHash(hash));
        }
        else
        {
            for (auto it = range.first; it != range.second; ++it)
            {
                const auto requester = it->second;
                m_arrivedResources[requester].push_back(resource);
            }
            m_requestedResources.erase(range.first, range.second);
        }
    }

    void ResourceComponent::resourceHasBeenLoadedFromFile(IResource* loadedResource, uint32_t size)
    {
        PlatformGuard guard(m_frameworkLock);
        m_bytesScheduledForLoading -= size;

        handleArrivedResource(m_resourceStorage.manageResource(*loadedResource, true));
    }

    void ResourceComponent::sendResourcesFromFile(const std::vector<IResource*>& loadedResources, uint64_t bytesLoaded, const Guid& requesterId)
    {
        PlatformGuard guard(m_frameworkLock);
        ManagedResourceVector managedResources;
        managedResources.reserve(loadedResources.size());
        for (const auto& loadedResource : loadedResources)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::sendResource: " << StringUtils::HexFromResourceContentHash(loadedResource->getHash()) << " name: " << loadedResource->getName() << " to " << requesterId);
            managedResources.push_back(m_resourceStorage.manageResource(*loadedResource, true));
        }
        m_bytesScheduledForLoading -= bytesLoaded;
        m_communicationSystem.sendResources(requesterId, managedResources);
    }

    void ResourceComponent::LoadResourcesFromFileTask::execute()
    {
        LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::LoadResourcesFromFileTask::execute: Loading resource data asynchronous from file");

        std::sort(m_resourcesToLoad.begin(), m_resourcesToLoad.end());

        const auto startTime = PlatformTime::GetMillisecondsMonotonic();

        struct NetworkResourceInfo {
            std::vector<IResource*> resources;
            uint64_t accumulatedFileSize;
        };
        HashMap<Guid, NetworkResourceInfo> resourceToSendViaNetwork(m_resourcesToLoad.size());
        for(const auto& resInfo : m_resourcesToLoad)
        {
            IResource* res = ResourcePersistation::RetrieveResourceFromStream(*resInfo.resourceStream, resInfo.fileEntry);
            if (!res)
            {
                LOG_ERROR(CONTEXT_FRAMEWORK, "Unable to load resource of type " << EnumToString(resInfo.fileEntry.resourceInfo.type)
                    << " at offset " << resInfo.fileEntry.offsetInBytes);
                return;
            }

            const Guid& requesterId = resInfo.requesterId;

            LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::LoadResourcesFromFileTask::execute: loaded " << EnumToString(resInfo.fileEntry.resourceInfo.type) << " Name: "
                << res->getName() << " Hash: " << resInfo.fileEntry.resourceInfo.hash << " Size " << resInfo.fileEntry.sizeInBytes << " Requester " << requesterId);

            m_resourceComponent.m_statistics.statResourcesLoadedFromFileNumber.incCounter(1);
            m_resourceComponent.m_statistics.statResourcesLoadedFromFileSize.incCounter(resInfo.fileEntry.sizeInBytes);

            if (requesterId.isInvalid())
            {
                m_resourceComponent.resourceHasBeenLoadedFromFile(res, resInfo.fileEntry.sizeInBytes);
            }
            else
            {
                NetworkResourceInfo& nri = resourceToSendViaNetwork[requesterId];
                nri.resources.push_back(res);
                nri.accumulatedFileSize += resInfo.fileEntry.sizeInBytes;
            }
        }
        const auto endTime = PlatformTime::GetMillisecondsMonotonic();
        if (endTime - m_taskCreationTime > 300)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::LoadResourcesFromFileTask::execute: Needed more than " << (endTime - m_taskCreationTime) <<
                "ms from query to load. Loaded " << m_resourcesToLoad.size() << " resources. Only load time " << (endTime - startTime) << "ms");
        }
        LOG_INFO_F(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
                    sos << "ResourceComponent::LoadResourcesFromFileTask::execute: " << m_resourcesToLoad.size() << " resources. tLoad " << (endTime - startTime) << "ms tQueue " << (endTime - m_taskCreationTime) << "ms";
                }));

        LOG_TRACE_F(CONTEXT_FRAMEWORK, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "ResourceComponent::LoadResourcesFromFileTask::execute: loading resources from file:";
            for (const auto& entry : m_resourcesToLoad)
            {
                sos << entry.fileEntry.resourceInfo.hash << ":" << entry.fileEntry.sizeInBytes << ":" << (entry.requesterId.isInvalid() ? "L" : "R") << " ";
            }
        }));

        for (const auto& p : resourceToSendViaNetwork)
        {
            m_resourceComponent.sendResourcesFromFile(p.value.resources, p.value.accumulatedFileSize, p.key);
        }
    }

    void ResourceComponent::newParticipantHasConnected(const Guid& guid)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceDeserializers.put(guid, new ResourceStreamDeserializer);
    }

    void ResourceComponent::participantHasDisconnected(const Guid& guid)
    {
        PlatformGuard guard(m_frameworkLock);
        // remove resource deserializer
        auto it = m_resourceDeserializers.find(guid);
        if (it != m_resourceDeserializers.end())
        {
            if (!it->value->processingFinished())
            {
                LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::participantHasDisconnected: Drop partially received resources from " << guid);
            }

            delete it->value;
            m_resourceDeserializers.remove(it);
        }
    }

    ManagedResource ResourceComponent::forceLoadResource(const ResourceContentHash& hash)
    {
        BinaryFileInputStream* resourceStream(0);
        ResourceFileEntry entry;
        const EStatus canLoadFromFile = m_resourceFiles.getEntry(hash, resourceStream, entry);
        if (canLoadFromFile == EStatus_RAMSES_OK)
        {
            m_statistics.statResourcesLoadedFromFileNumber.incCounter(1);
            m_statistics.statResourcesLoadedFromFileSize.incCounter(entry.sizeInBytes);

            IResource* lowLevelResource = ResourcePersistation::RetrieveResourceFromStream(*resourceStream, entry);
            return m_resourceStorage.manageResource(*lowLevelResource, true);
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::forceLoadResource: Could not find or load requested resource: " << StringUtils::HexFromResourceContentHash(hash));
            return ManagedResource();
        }
    }

    bool ResourceComponent::isReceivingFromParticipant(const Guid& participant) const
    {
        ResourceStreamDeserializer** deser = m_resourceDeserializers.get(participant);
        return deser && !(*deser)->processingFinished();
    }

    bool ResourceComponent::hasRequestForResource(ResourceContentHash hash, RequesterID requester) const
    {
        auto range = m_requestedResources.equal_range(hash);
        return FindInRequestsRange(range, requester) != range.second;
    }

    void ResourceComponent::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceStorage.reserveResourceCount(totalCount);
    }

    void ResourceComponent::handleResourcesNotAvailable(const ResourceContentHashVector& resources, const Guid& providerID)
    {
        for (const auto& hash : resources)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "RemoteResourceSource::handleResourcesNotAvailable: Requested resource hash " << hash << " is not available at " << providerID);

            // remove all requests for hash
            m_requestedResources.erase(hash);

            // TODO(tobias) maybe request by another provider if there is one
        }
    }

    ResourceComponent::RequestsMap::const_iterator ResourceComponent::FindInRequestsRange(std::pair<RequestsMap::const_iterator, RequestsMap::const_iterator> range, RequesterID requester)
    {
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == requester)
            {
                return it;
            }
        }
        return range.second;
    }
}
