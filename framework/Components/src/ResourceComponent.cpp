//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceComponent.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/ResourceFilesRegistry.h"
#include "TaskFramework/ITaskQueue.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/StringUtils.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryInputStream.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "TransportCommon/SceneUpdateStreamDeserializer.h"
#include "TransportCommon/SceneUpdateSerializer.h"
#include "Components/SceneUpdate.h"
#include <algorithm>

namespace ramses_internal
{
    ResourceComponent::ResourceComponent(ITaskQueue& queue, const Guid& myAddress, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier,
        StatisticCollectionFramework& statistics, PlatformLock& frameworkLock, uint32_t maximumTotalBytesForAsynResourceLoading)
        : m_frameworkLock(frameworkLock)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_resourceStorage(frameworkLock, statistics)
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
        m_communicationSystem.setResourceConsumerServiceHandler(nullptr);
        m_communicationSystem.setResourceProviderServiceHandler(nullptr);

        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);
        m_taskQueueForResourceLoading.disableAcceptingTasksAfterExecutingCurrentQueue();
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

    ramses_internal::ManagedResource ResourceComponent::manageResource(const IResource& resource, bool deletionAllowed)
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

    void ResourceComponent::handleRequestResources(const ResourceContentHashVector& ids, const Guid& requesterId)
    {
        ManagedResourceVector resourceToSendViaNetwork;
        ResourceContentHashVector unavailableResources;
        ResourceContentHashVector resourcesToBeLoaded;

        for(const auto& id : ids)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: " << id << " from " << requesterId);
            const ManagedResource& resource = m_resourceStorage.getResource(id);
            if (resource)
            {
                LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: sendResource" << id << " name: " << resource->getName());
                resourceToSendViaNetwork.push_back(resource);
            }
            else
            {
                // try resource files
                ResourceLoadInfo loadInfo;
                const EStatus canLoadFromFile = m_resourceFiles.getEntry(id, loadInfo.resourceStream, loadInfo.fileEntry);
                if (canLoadFromFile == EStatus::Ok)
                {
                    loadInfo.requesterId = requesterId;
                    m_resourcesToBeLoaded.push_back(loadInfo);
                    resourcesToBeLoaded.push_back(id);
                }
                else
                {
                    unavailableResources.push_back(id);
                    LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::handleResourceRequest: Could not find or load requested resource: " << id << " requested by " << requesterId);
                }
            }
        }

        if (!resourceToSendViaNetwork.empty())
        {
            m_statistics.statResourcesSentNumber.incCounter(static_cast<UInt32>(resourceToSendViaNetwork.size()));
            for (auto& res : resourceToSendViaNetwork)
                res->compress(IResource::CompressionLevel::REALTIME);
            SceneUpdate update{SceneActionCollection(), resourceToSendViaNetwork};
            m_communicationSystem.sendResources(requesterId, SceneUpdateSerializer(update));
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
                    sos << res->getHash() << " ";
                sos << "; ";
            }
            if (!resourcesToBeLoaded.empty())
            {
                sos << "load " << resourcesToBeLoaded.size() << " from file ";
                for (const auto& hash : resourcesToBeLoaded)
                    sos << hash << " ";
                sos << "; ";
            }
            if (!unavailableResources.empty())
            {
                sos << "send " << unavailableResources.size() << " unavailable ";
                for (const auto& hash : unavailableResources)
                    sos << hash << " ";
            }
        }));

        triggerLoadingResourcesFromFile();
    }

    void ResourceComponent::requestResourceAsynchronouslyFromFramework(const ResourceContentHashVector& resourceHashes, const ResourceRequesterID& requesterID, const Guid& providerID)
    {
        ResourceContentHashVector resourcesToBeRetrievedFromProvider;
        ResourceContentHashVector resourcesToBeLoaded;
        ResourceContentHashVector resourcesLocallyAvailable;

        for (const auto hash : resourceHashes)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::requestResourceAsynchronouslyFromFramework:" << hash);
            ManagedResource managedResource = m_resourceStorage.getResource(hash);

            if (managedResource)
            {
                // already available
                resourcesLocallyAvailable.push_back(hash);
                m_arrivedResources[requesterID].push_back(managedResource);
            }
            else
            {
                const auto range = m_requestedResources.equal_range(hash);
                if (FindInRequestsRange(range, requesterID) == range.second)
                {
                    // add to requests if not yet requested by this requester
                    m_requestedResources.insert(std::make_pair(hash, requesterID));
                }

                {
                    // only trigger request from file or network if not already requested by any requester
                    ResourceLoadInfo loadInfo;
                    const EStatus canLoadResource = m_resourceFiles.getEntry(hash, loadInfo.resourceStream, loadInfo.fileEntry);
                    if (canLoadResource == EStatus::Ok)
                    {
                        m_resourcesToBeLoaded.push_back(loadInfo);
                        LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceComponent::requestResourceAsynchronouslyFromFramework: resource found in resource file, will be loaded later:" << hash);
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
                    sos << hash << " ";
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
                    sos << hash << " ";
                sos << "; ";
            }
            if (!resourcesToBeLoaded.empty())
            {
                sos << "load from file " << resourcesToBeLoaded.size() << " resources: ";
                for (const auto& hash : resourcesToBeLoaded)
                    sos << hash << " ";
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

    void ResourceComponent::cancelResourceRequest(const ResourceContentHash& resourceHash, const ResourceRequesterID& requesterID)
    {
        // remove from arrived
        bool removedFromArrived = false;
        auto& arrivedForRequester = m_arrivedResources[requesterID];
        for (auto it = arrivedForRequester.begin(); it != arrivedForRequester.end(); ++it)
        {
            if ((*it)->getHash() == resourceHash)
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

    ManagedResourceVector ResourceComponent::popArrivedResources(const ResourceRequesterID& requesterID)
    {
        ManagedResourceVector res;
        m_arrivedResources[requesterID].swap(res);
        if (!res.empty() && m_hasArrivedRemoteResources[requesterID])
        {
            m_hasArrivedRemoteResources[requesterID] = false;
            LOG_INFO(CONTEXT_FRAMEWORK, "ResourceComponent::popArrivedResources: " << res.size() << " resources (>0 remote) have arrived for requester " << requesterID);
        }
        return res;
    }

    void ResourceComponent::addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ramses_internal::ResourceTableOfContents& toc)
    {
        for (const auto& item : toc.getFileContents())
        {
            storeResourceInfo(item.key, item.value.resourceInfo);
        }
        m_resourceFiles.registerResourceFile(resourceFileInputStream, toc, m_resourceStorage);
    }

    bool ResourceComponent::hasResourceFile(const String& resourceFileName) const
    {
        return m_resourceFiles.hasResourceFile(resourceFileName);
    }

    void ResourceComponent::forceLoadFromResourceFile(const String& resourceFileName)
    {
        // If a resources of a file are force loaded, check if they are in use by any scene object (=hashusage) or as a resource
        // a) If they are in use, we need to load them from file, also remove the deletion allowed flag from
        // them, because they is not supposed to be loadable anymore.
        // b) If a resource is unused, nothing is to be done since there wouldn't be any entry in the resource storage for it
        const FileContentsMap* content = m_resourceFiles.getContentsOfResourceFile(resourceFileName);
        if (!content)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::forceLoadFromResourceFile: " << resourceFileName << " unknown, can't force load");
            return;
        }

        for (auto const& entry : *content)
        {
            auto const& id = entry.key;
            if (m_resourceStorage.isFileResourceInUseAnywhereElse(id))
            {
                ManagedResource res;
                if (!m_resourceStorage.getResource(id))
                    res = forceLoadResource(id);

                m_resourceStorage.markDeletionDisallowed(id);
            }
        }
    }

    void ResourceComponent::removeResourceFile(const String& resourceFileName)
    {
        m_resourceFiles.unregisterResourceFile(resourceFileName);
    }

    void ResourceComponent::handleSendResource(absl::Span<const Byte> receivedResourceData, const Guid& providerID)
    {
        PlatformGuard guard(m_frameworkLock);
        auto it = m_resourceDeserializers.find(providerID);
        if (it == m_resourceDeserializers.end())
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::handleSendResource: from unknown provider " << providerID);
            return;
        }
        SceneUpdateStreamDeserializer* deserializer = it->second.get();
        assert(deserializer);

        auto result = deserializer->processData(receivedResourceData);
        switch (result.result)
        {
        case SceneUpdateStreamDeserializer::ResultType::Empty:
            break;
        case SceneUpdateStreamDeserializer::ResultType::Failed:
            LOG_ERROR(CONTEXT_RENDERER, "ResourceComponent::handleSendResource: deserialization failed from provider:" << providerID);
            // TODO(tobias) handle properly by disconnect participant
            break;
        case SceneUpdateStreamDeserializer::ResultType::HasData:
            {
                if (result.resources.empty())
                    LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::handleSendResource: no resources deserialized from " << providerID << ", size " << receivedResourceData.size());
                for (auto& res : result.resources)
                    handleArrivedResource(m_resourceStorage.manageResource(*res.release(), false));
            }
        }

        if (!m_unrequestedResources.empty())
        {
            LOG_INFO_F(CONTEXT_FRAMEWORK, ([this](ramses_internal::StringOutputStream& sos) {
                sos << "ResourceComponent::handleArrivedResource: Received unrequested resources with hash:";
                for (const auto hash : m_unrequestedResources)
                    sos << " " << hash;
            }));
            m_unrequestedResources.clear();
        }
    }

    void ResourceComponent::handleArrivedResource(const ManagedResource& resource)
    {
        const ResourceContentHash hash = resource->getHash();

        LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::handleArrivedResource: resource available " << hash);

        const auto range = m_requestedResources.equal_range(hash);
        if (range.first == range.second)
            m_unrequestedResources.push_back(hash);
        else
        {
            for (auto it = range.first; it != range.second; ++it)
            {
                const auto requester = it->second;
                m_arrivedResources[requester].push_back(resource);
                m_hasArrivedRemoteResources[requester] = true;
                m_statistics.statResourcesReceivedNumber.incCounter(1);
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
            LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceComponent::sendResource: " << loadedResource->getHash() << " name: " << loadedResource->getName() << " to " << requesterId);
            managedResources.push_back(m_resourceStorage.manageResource(*loadedResource, true));
        }
        m_bytesScheduledForLoading -= bytesLoaded;

        for (auto& res : managedResources)
            res->compress(IResource::CompressionLevel::REALTIME);
        SceneUpdate update{SceneActionCollection(), managedResources};
        m_communicationSystem.sendResources(requesterId, SceneUpdateSerializer(update));
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
                // always decompress locally requested resources in load thread (not later in renderer thread)
                res->decompress();
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
        m_resourceDeserializers[guid] = std::make_unique<SceneUpdateStreamDeserializer>();
    }

    void ResourceComponent::participantHasDisconnected(const Guid& guid)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceDeserializers.erase(guid);
    }

    ManagedResource ResourceComponent::forceLoadResource(const ResourceContentHash& hash)
    {
        BinaryFileInputStream* resourceStream(nullptr);
        ResourceFileEntry entry;
        const EStatus canLoadFromFile = m_resourceFiles.getEntry(hash, resourceStream, entry);
        if (canLoadFromFile == EStatus::Ok)
        {
            m_statistics.statResourcesLoadedFromFileNumber.incCounter(1);
            m_statistics.statResourcesLoadedFromFileSize.incCounter(entry.sizeInBytes);

            IResource* lowLevelResource = ResourcePersistation::RetrieveResourceFromStream(*resourceStream, entry);
            return m_resourceStorage.manageResource(*lowLevelResource, true);
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::forceLoadResource: Could not find or load requested resource: " << hash);
            return ManagedResource();
        }
    }

    bool ResourceComponent::hasDeserializerForParticipant(const Guid& participant) const
    {
        return m_resourceDeserializers.find(participant) != m_resourceDeserializers.end();
    }

    bool ResourceComponent::hasRequestForResource(ResourceContentHash hash, ResourceRequesterID requester) const
    {
        auto range = m_requestedResources.equal_range(hash);
        return FindInRequestsRange(range, requester) != range.second;
    }

    void ResourceComponent::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceStorage.reserveResourceCount(totalCount);
    }

    ramses_internal::ManagedResourceVector ResourceComponent::resolveResources(ResourceContentHashVector& hashes)
    {
        ManagedResourceVector result;
        for (auto& ressourcehash : hashes)
        {
            ManagedResource mr = getResource(ressourcehash);
            if (!mr)
                mr = forceLoadResource(ressourcehash);
            assert(mr);
            result.push_back(mr);
        }
        return result;
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

    ResourceComponent::RequestsMap::const_iterator ResourceComponent::FindInRequestsRange(std::pair<RequestsMap::const_iterator, RequestsMap::const_iterator> range, ResourceRequesterID requester)
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
