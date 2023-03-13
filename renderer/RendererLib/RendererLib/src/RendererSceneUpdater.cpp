//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/SceneActionApplier.h"
#include "Scene/ResourceChanges.h"
#include "SceneUtils/ResourceUtils.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/IRendererResourceCache.h"
#include "RendererLib/RendererCachedScene.h"
#include "RendererLib/Renderer.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/SceneStateExecutor.h"
#include "RendererLib/RendererResourceManager.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/DataLinkUtils.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/EmbeddedCompositingManager.h"
#include "RendererLib/PendingSceneResourcesUtils.h"
#include "RendererLib/RendererLogger.h"
#include "RendererLib/IntersectionUtils.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RendererLib/ResourceUploader.h"
#include "RendererEventCollector.h"
#include "Components/FlushTimeInformation.h"
#include "Components/SceneUpdate.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Image.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/Macros.h"
#include <algorithm>
#include "RendererLib/SceneResourceUploader.h"

namespace ramses_internal
{
    RendererSceneUpdater::RendererSceneUpdater(
        DisplayHandle display,
        IPlatform& platform,
        Renderer& renderer,
        RendererScenes& rendererScenes,
        SceneStateExecutor& sceneStateExecutor,
        RendererEventCollector& eventCollector,
        FrameTimer& frameTimer,
        SceneExpirationMonitor& expirationMonitor,
        IThreadAliveNotifier& notifier,
        IRendererResourceCache* rendererResourceCache
        )
        : m_display{ display }
        , m_platform(platform)
        , m_renderer(renderer)
        , m_rendererScenes(rendererScenes)
        , m_sceneStateExecutor(sceneStateExecutor)
        , m_rendererEventCollector(eventCollector)
        , m_frameTimer(frameTimer)
        , m_expirationMonitor(expirationMonitor)
        , m_rendererResourceCache(rendererResourceCache)
        , m_notifier(notifier)
    {
    }

    RendererSceneUpdater::~RendererSceneUpdater()
    {
        while (0u != m_rendererScenes.size())
        {
            const SceneId sceneId = m_rendererScenes.begin()->key;
            destroyScene(sceneId);
        }
        assert(m_scenesToBeMapped.size() == 0u);

        if (m_displayResourceManager)
            destroyDisplayContext();
    }

    void RendererSceneUpdater::handleSceneUpdate(SceneId sceneId, SceneUpdate&& sceneUpdate)
    {
        ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneId);

        LOG_TRACE_P(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneUpdate: for sceneId {}, flushCounter {}, sceneState {}",
                    sceneId, sceneUpdate.flushInfos.flushCounter, EnumToString(sceneState));

        if (sceneState == ESceneState::SubscriptionPending)
        {
            // initial content of scene arrived, scene can be set from pending to subscribed
            if (m_sceneStateExecutor.checkIfCanBeSubscribed(sceneId))
            {
                m_sceneStateExecutor.setSubscribed(sceneId);
                sceneState = m_sceneStateExecutor.getSceneState(sceneId);
                assert(sceneState == ESceneState::Subscribed);
            }
        }

        if (SceneStateIsAtLeast(sceneState, ESceneState::Subscribed))
            consolidatePendingSceneActions(sceneId, std::move(sceneUpdate));
        else
            LOG_ERROR_P(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneActions could not apply scene actions because scene {} is neither subscribed nor mapped", sceneId);
    }

    void RendererSceneUpdater::createDisplayContext(const DisplayConfig& displayConfig, IBinaryShaderCache* binaryShaderCache)
    {
        assert(!m_displayResourceManager);
        assert(!m_asyncEffectUploader);
        m_renderer.resetRenderInterruptState();
        m_renderer.createDisplayContext(displayConfig);

        if (m_renderer.hasDisplayController())
        {
            IDisplayController& displayController = m_renderer.getDisplayController();
            IRenderBackend& renderBackend = displayController.getRenderBackend();
            IEmbeddedCompositingManager& embeddedCompositingManager = displayController.getEmbeddedCompositingManager();

            m_asyncEffectUploader = std::make_unique<AsyncEffectUploader>(m_platform, renderBackend, m_notifier, static_cast<int>(m_display.asMemoryHandle()));
            if (!m_asyncEffectUploader->createResourceUploadRenderBackendAndStartThread())
            {
                m_renderer.destroyDisplayContext();
                m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayCreateFailed, m_display);
                return;
            }
            // ownership of uploadStrategy is transferred into RendererResourceManager
            m_displayResourceManager = createResourceManager(renderBackend,
                                                        embeddedCompositingManager,
                                                        displayConfig,
                                                        binaryShaderCache);

            m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayCreated, m_display);

            LOG_INFO_P(CONTEXT_RENDERER, "Created display: {}x{}{} MSAA{}",
                displayController.getDisplayWidth(), displayController.getDisplayHeight(), (displayConfig.getFullscreenState() ? " fullscreen" : ""),
                displayConfig.getAntialiasingSampleCount());
        }
        else
            m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayCreateFailed, m_display);
    }

    std::unique_ptr<IRendererResourceManager> RendererSceneUpdater::createResourceManager(
        IRenderBackend& renderBackend,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        const DisplayConfig& displayConfig,
        IBinaryShaderCache* binaryShaderCache)
    {
        return std::make_unique<RendererResourceManager>(
            renderBackend,
            std::make_unique<ResourceUploader>(displayConfig.isAsyncEffectUploadEnabled(), binaryShaderCache),
            *m_asyncEffectUploader,
            embeddedCompositingManager,
            displayConfig,
            m_frameTimer,
            m_renderer.getStatistics());
    }

    void RendererSceneUpdater::destroyResourceManager()
    {
        m_displayResourceManager.reset();
    }

    void RendererSceneUpdater::destroyDisplayContext()
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::destroyDisplayContext cannot destroy display context which was not created.");
            m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayDestroyFailed, m_display);
            return;
        }

        // are there any scenes waiting to be mapped or mapping/mapped/rendered
        bool displayHasMappedScene = !m_scenesToBeMapped.empty();
        for (const auto& it : m_rendererScenes)
        {
            if (m_sceneStateExecutor.getSceneState(it.key) >= ESceneState::MappingAndUploading)
                displayHasMappedScene = true;
        }

        if (displayHasMappedScene)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::destroyDisplayContext cannot destroy display, there is one or more scenes mapped (or being mapped) to it, unmap all scenes from it first.");
            m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayDestroyFailed, m_display);
            return;
        }

        m_asyncEffectUploader->destroyResourceUploadRenderBackendAndStopThread();
        m_asyncEffectUploader.reset();
        destroyResourceManager();

        m_renderer.resetRenderInterruptState();
        m_renderer.destroyDisplayContext();
        assert(!m_renderer.hasDisplayController());
        m_rendererEventCollector.addDisplayEvent(ERendererEventType::DisplayDestroyed, m_display);
    }

    void RendererSceneUpdater::updateScenes()
    {
        {
            m_renderer.m_traceId = 1;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes request resources from network, upload used resources and unload obsolete resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateClientResources);
            requestAndUploadAndUnloadResources();
        }

        {
            m_renderer.m_traceId = 2;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes try to apply pending flushes, only apply sync flushes if all resources available");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::ApplySceneActions);
            tryToApplyPendingFlushes();
        }

        {
            m_renderer.m_traceId = 3;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes executing pending scene reference commands and updates states");
            assert(m_sceneReferenceLogic);
            m_sceneReferenceLogic->update();
        }

        {
            m_renderer.m_traceId = 4;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes unref obsolete client resources and upload pending scene resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateSceneResources);
            processStagedResourceChangesFromAppliedFlushes();
        }

        {
            m_renderer.m_traceId = 5;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update embedded compositing resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateEmbeddedCompositingResources);
            uploadUpdatedECStreams();
        }

        {
            m_renderer.m_traceId = 6;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes stream texture dirtiness");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateStreamTextures);
            handleECStreamAvailabilityChanges();
        }

        {
            m_renderer.m_traceId = 7;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes to be mapped/shown");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateScenesToBeMapped);
            updateScenesStates();
        }

        {
            m_renderer.m_traceId = 8;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes resource cache");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateResourceCache);
            updateScenesResourceCache();
            uploadAndUnloadVertexArrays();
        }

        {
            m_renderer.m_traceId = 9;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update shader animations");
            updateScenesShaderAnimations();
        }

        {
            m_renderer.m_traceId = 10;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes transformation cache and transformation links");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateTransformations);
            updateScenesTransformationCache();
        }

        {
            m_renderer.m_traceId = 11;
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes data links");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateDataLinks);
            updateScenesDataLinks();
        }

        m_renderer.m_traceId = 12;
        for (const auto scene : m_modifiedScenesToRerender)
        {
            if (m_sceneStateExecutor.getSceneState(scene) == ESceneState::Rendered)
                m_renderer.markBufferWithSceneForRerender(scene);
        }
        m_modifiedScenesToRerender.clear();

        if (!m_skipUnmodifiedScenes)
        {
            // Mark all shown scenes for re-render regardless if modified or not
            for (const auto& scene : m_rendererScenes)
            {
                if (m_sceneStateExecutor.getSceneState(scene.key) == ESceneState::Rendered)
                    m_renderer.markBufferWithSceneForRerender(scene.key);
            }
        }
    }

    void RendererSceneUpdater::logTooManyFlushesAndUnsubscribeIfRemoteScene(SceneId sceneId, std::size_t numPendingFlushes)
    {
        LOG_ERROR(CONTEXT_RENDERER, "Scene " << sceneId << " has " << numPendingFlushes << " pending flushes,"
            << " force applying pending flushes seems to have been interrupted too often and the renderer has no way to catch up without potentially blocking other scenes."
            << " Possible causes: too many flushes queued and couldn't be applied (even force-applied); or renderer thread was stopped or stalled,"
            << " e.g. because of taking screenshots, and couldn't process the flushes.");

        if (m_sceneStateExecutor.getScenePublicationMode(sceneId) != EScenePublicationMode_LocalOnly)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Force unsubscribing scene " << sceneId << " to avoid risk of running out of memory!"
                << " Any incoming data for the scene will be ignored till the scene is re-subscribed.");
            // Unsubscribe scene as 'indirect' because it is not triggered by user
            handleSceneUnsubscriptionRequest(sceneId, true);
        }
        else
        {
            // Don't force-ubsubscribe local scenes
            // Local client is responsible for his own scene - should not spam the renderer with flushes, or if he does
            // and renderer goes out of memory -> it is possible to fix on client side in the local case
            LOG_ERROR(CONTEXT_RENDERER, "Because scene " << sceneId << " is a local scene, it will not be forcefully ubsubscribed. Beware of possible out-of-memory errors!");
        }
    }

    void RendererSceneUpdater::consolidatePendingSceneActions(SceneId sceneID, SceneUpdate&& sceneUpdate)
    {
        StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);
        auto& pendingData = stagingInfo.pendingData;
        auto& pendingFlushes = pendingData.pendingFlushes;
        pendingFlushes.emplace_back();
        PendingFlush& flushInfo = pendingFlushes.back();

        const UInt32 numActions = sceneUpdate.actions.numberOfActions();
        flushInfo.flushIndex = sceneUpdate.flushInfos.flushCounter;
        ResourceChanges& resourceChanges = sceneUpdate.flushInfos.resourceChanges;

        if (sceneUpdate.flushInfos.hasSizeInfo)
            stagingInfo.sizeInformation = sceneUpdate.flushInfos.sizeInfo;
        pendingData.sceneReferenceActions.insert(pendingData.sceneReferenceActions.end(), sceneUpdate.flushInfos.sceneReferences.cbegin(), sceneUpdate.flushInfos.sceneReferences.cend());
        flushInfo.timeInfo = sceneUpdate.flushInfos.flushTimeInfo;
        flushInfo.versionTag = sceneUpdate.flushInfos.versionTag;

        // get ptp synchronized time and check current and received times for validity
        std::chrono::milliseconds flushLatencyMs{ 0 };
        if (flushInfo.timeInfo.internalTimestamp != FlushTime::InvalidTimestamp)
        {
            // collect latency timing statistics between flush call on Scene and here
            const auto flushConsolidateTs = FlushTime::Clock::now();
            if (flushConsolidateTs != FlushTime::InvalidTimestamp)
                flushLatencyMs = std::chrono::duration_cast<std::chrono::milliseconds>(flushConsolidateTs - flushInfo.timeInfo.internalTimestamp);
        }

        m_renderer.getStatistics().trackArrivedFlush(sceneID, numActions, resourceChanges.m_resourcesAdded.size(), resourceChanges.m_resourcesRemoved.size(), resourceChanges.m_sceneResourceActions.size(), flushLatencyMs);

        LOG_TRACE_F(CONTEXT_RENDERER, ([&](StringOutputStream& logStream) {
            logStream << "Flush " << flushInfo.flushIndex << " for scene " << sceneID << " arrived (latency " << flushLatencyMs.count() << ") ";
            logStream << "[actions:" << numActions << "(" << sceneUpdate.actions.collectionData().size() << " bytes)]";
            logStream << "[addRefs res (" << resourceChanges.m_resourcesAdded.size() << "):";
            for (const auto& hash : resourceChanges.m_resourcesAdded)
                logStream << " " << hash;
            logStream << "]";
            logStream << "[removeRefs res (" << resourceChanges.m_resourcesRemoved.size() << "):";
            for (const auto& hash : resourceChanges.m_resourcesRemoved)
                logStream << " " << hash;
            logStream << "]";
            logStream << "[scene res actions:" << resourceChanges.m_sceneResourceActions.size() << "]";
            if (sceneUpdate.flushInfos.hasSizeInfo)
            {
                logStream << " " << stagingInfo.sizeInformation;
            }
        }));
        if (!sceneUpdate.flushInfos.resourceChanges.empty())
        {
            LOG_TRACE(CONTEXT_RENDERER, sceneUpdate.flushInfos.resourceChanges);
        }

        PendingSceneResourcesUtils::ConsolidateSceneResourceActions(resourceChanges.m_sceneResourceActions, pendingData.sceneResourceActions);

        assert(sceneUpdate.resources.size() == resourceChanges.m_resourcesAdded.size());
        assert(std::equal(sceneUpdate.resources.cbegin(), sceneUpdate.resources.cend(), resourceChanges.m_resourcesAdded.cbegin(),
            [](const auto& mr, const auto& hash) { return mr->getHash() == hash; }));
        flushInfo.resourceDataToProvide = std::move(sceneUpdate.resources);
        flushInfo.resourcesAdded = std::move(resourceChanges.m_resourcesAdded);
        flushInfo.resourcesRemoved = std::move(resourceChanges.m_resourcesRemoved);
        flushInfo.sceneActions = std::move(sceneUpdate.actions);

        if (stagingInfo.pendingData.pendingFlushes.size() > m_maximumPendingFlushesToKillScene)
        {
            const auto numPendingFlushes = getNumberOfPendingNonEmptyFlushes(sceneID);
            if (numPendingFlushes > m_maximumPendingFlushesToKillScene)
                logTooManyFlushesAndUnsubscribeIfRemoteScene(sceneID, numPendingFlushes);
        }
    }

    void RendererSceneUpdater::consolidateResourceDataForMapping(SceneId sceneID)
    {
        // consolidate resources from pending flushes into staging data for mapping
        StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);
        auto& resourcesForMapping = stagingInfo.resourcesToUploadOnceMapping;
        for (auto& pendingFlush : stagingInfo.pendingData.pendingFlushes)
        {
            // remove no more needed resources
            if (!pendingFlush.resourcesRemoved.empty())
            {
                auto it = std::remove_if(resourcesForMapping.begin(), resourcesForMapping.end(), [&](const auto& mr)
                {
                    const auto mrHash = mr->getHash();
                    return std::find(std::cbegin(pendingFlush.resourcesRemoved), std::cend(pendingFlush.resourcesRemoved), mrHash) != pendingFlush.resourcesRemoved.cend();
                });
                resourcesForMapping.erase(it, resourcesForMapping.end());
            }
            // add newly needed resources
            resourcesForMapping.insert(resourcesForMapping.end(), pendingFlush.resourceDataToProvide.cbegin(), pendingFlush.resourceDataToProvide.cend());
            pendingFlush.resourceDataToProvide.clear();

            // assert stored resources are unique (without modifying state!)
            assert([&resourcesForMapping]()
            {
                ResourceContentHashVector hashes{ resourcesForMapping.size() };
                std::transform(resourcesForMapping.cbegin(), resourcesForMapping.cend(), hashes.begin(), [](const auto& mr) { return mr->getHash(); });
                std::sort(hashes.begin(), hashes.end());
                return std::unique(hashes.begin(), hashes.end()) == hashes.end();
            }());
        }
    }

    void RendererSceneUpdater::referenceAndProvidePendingResourceData(SceneId sceneID)
    {
        // collect and provide resource data to resource manager
        assert(m_displayResourceManager);
        std::vector<ManagedResourceVector*> resourcesToProvide;

        // collect from staged data for scene to be mapped
        StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);
        if (!stagingInfo.resourcesToUploadOnceMapping.empty())
        {
            auto& resDataToMap = stagingInfo.resourcesToUploadOnceMapping;
            ResourceContentHashVector hashesToMap{ resDataToMap.size() };
            std::transform(resDataToMap.cbegin(), resDataToMap.cend(), hashesToMap.begin(), [](const auto& mr) { return mr->getHash(); });
            m_displayResourceManager->referenceResourcesForScene(sceneID, hashesToMap);
            resourcesToProvide.push_back(&resDataToMap);
        }

        // collect from pending flushes
        for (auto& pendingFlush : stagingInfo.pendingData.pendingFlushes)
        {
            if (!pendingFlush.resourceDataToProvide.empty())
            {
                m_displayResourceManager->referenceResourcesForScene(sceneID, pendingFlush.resourcesAdded);
                resourcesToProvide.push_back(&pendingFlush.resourceDataToProvide);
            }
        }

        // provide all collected resource data
        for (auto resList : resourcesToProvide)
        {
            for (const auto& mr : *resList)
            {
                m_displayResourceManager->provideResourceData(mr);
                uint32_t dummySize = 0;
                if (m_rendererResourceCache && !m_rendererResourceCache->hasResource(mr->getHash(), dummySize))
                    RendererResourceManagerUtils::StoreResource(m_rendererResourceCache, mr.get(), sceneID);
            }
            // data was provided, clear shared_ptr references from list
            resList->clear();
        }
    }

    void RendererSceneUpdater::requestAndUploadAndUnloadResources()
    {
        if (!m_displayResourceManager)
            return;

        for (const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            if (m_sceneStateExecutor.getSceneState(sceneID) < ESceneState::MappingAndUploading)
                consolidateResourceDataForMapping(sceneID);
            else
                referenceAndProvidePendingResourceData(sceneID);
        }

        // if there are resources to upload, unload and upload pending resources
        if (m_displayResourceManager->hasResourcesToBeUploaded())
            m_displayResourceManager->uploadAndUnloadPendingResources();
    }

    void RendererSceneUpdater::uploadUpdatedECStreams()
    {
        if (!m_renderer.hasDisplayController())
            return;

        IEmbeddedCompositingManager& embeddedCompositingManager = m_renderer.getDisplayController().getEmbeddedCompositingManager();
        if (embeddedCompositingManager.hasRealCompositor())
        {
            embeddedCompositingManager.processClientRequests();
            if (embeddedCompositingManager.hasUpdatedContentFromStreamSourcesToUpload())
            {
                m_streamUpdates.clear();
                embeddedCompositingManager.uploadResourcesAndGetUpdates(m_streamUpdates);

                const auto& texLinks = m_rendererScenes.getSceneLinksManager().getTextureLinkManager();
                StreamBufferLinkVector links;
                for (const auto& updatedSource : m_streamUpdates)
                {
                    m_renderer.getStatistics().streamTextureUpdated(updatedSource.first, updatedSource.second);
                    const auto& streamUsage = m_displayResourceManager->getStreamUsage(updatedSource.first);
                    // mark all scenes linked as consumer to updated source as modified
                    for (const auto streamBuffer : streamUsage)
                    {
                        links.clear();
                        texLinks.getStreamBufferLinks().getLinkedConsumers(streamBuffer, links);
                        for (const auto& link : links)
                            m_modifiedScenesToRerender.put(link.consumerSceneId);
                    }
                }
            }
        }
    }

    void RendererSceneUpdater::tryToApplyPendingFlushes()
    {
        // check and try to apply pending flushes
        for(const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);

            if (!stagingInfo.pendingData.pendingFlushes.empty())
                updateScenePendingFlushes(sceneID, stagingInfo);
        }
    }

    void RendererSceneUpdater::updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo)
    {
        const ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneID);
        const Bool sceneIsRenderedOrRequested = (sceneState == ESceneState::Rendered || sceneState == ESceneState::RenderRequested); // requested can become rendered still in this frame
        const Bool sceneIsMapped = (sceneState == ESceneState::Mapped) || sceneIsRenderedOrRequested;
        const Bool sceneIsMappedOrMapping = (sceneState == ESceneState::MappingAndUploading) || sceneIsMapped;
        const Bool resourcesReady = sceneIsMappedOrMapping && areResourcesFromPendingFlushesUploaded(sceneID);

        Bool canApplyFlushes = !sceneIsMappedOrMapping || resourcesReady;

        if (sceneIsRenderedOrRequested && m_renderer.hasAnyBufferWithInterruptedRendering())
            canApplyFlushes &= !m_renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneID);

        if (!canApplyFlushes && sceneIsMapped && stagingInfo.pendingData.pendingFlushes.size() > m_maximumPendingFlushes)
        {
            const auto numPendingFlushes = getNumberOfPendingNonEmptyFlushes(sceneID);
            if (numPendingFlushes > m_maximumPendingFlushes)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Force applying pending flushes! Scene " << sceneID << " has " << numPendingFlushes << " pending flushes, renderer cannot catch up with resource updates.");
                logMissingResources(stagingInfo.pendingData, sceneID);

                canApplyFlushes = true;
                m_renderer.resetRenderInterruptState();
            }
        }

        if (canApplyFlushes)
        {
            stagingInfo.pendingData.allPendingFlushesApplied = true;
            applyPendingFlushes(sceneID, stagingInfo);
        }
        else
            m_renderer.getStatistics().flushBlocked(sceneID);
    }

    void RendererSceneUpdater::applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo)
    {
        auto& rendererScene = const_cast<RendererCachedScene&>(m_rendererScenes.getScene(sceneID));
        rendererScene.preallocateSceneSize(stagingInfo.sizeInformation);

        PendingData& pendingData = stagingInfo.pendingData;
        PendingFlushes& pendingFlushes = pendingData.pendingFlushes;
        for (auto& pendingFlush : pendingFlushes)
        {
            const auto hadActiveShaderAnimation = rendererScene.hasActiveShaderAnimation();
            // re-enable skub optimization
            // skub will be disabled again if a semantic time uniform is applied during first rendering after flush
            rendererScene.setActiveShaderAnimation(false);
            if (pendingFlush.timeInfo.isEffectTimeSync)
            {
                LOG_INFO_P(CONTEXT_RENDERER, "EffectTimeSync: {} for scene: {}",
                    std::chrono::time_point_cast<std::chrono::milliseconds>(pendingFlush.timeInfo.internalTimestamp).time_since_epoch().count(),
                    rendererScene.getSceneId());
                rendererScene.setEffectTimeSync(pendingFlush.timeInfo.internalTimestamp);
            }
            applySceneActions(rendererScene, pendingFlush);

            if (pendingFlush.versionTag.isValid())
            {
                LOG_INFO(CONTEXT_SMOKETEST, "Named flush applied on scene " << rendererScene.getSceneId() <<
                    " with sceneVersionTag " << pendingFlush.versionTag);
                m_rendererEventCollector.addSceneFlushEvent(ERendererEventType::SceneFlushed, sceneID, pendingFlush.versionTag);
            }
            stagingInfo.lastAppliedVersionTag = pendingFlush.versionTag;
            m_expirationMonitor.onFlushApplied(sceneID, pendingFlush.timeInfo.expirationTimestamp, pendingFlush.versionTag, pendingFlush.flushIndex);
            m_renderer.getStatistics().flushApplied(sceneID);

            // mark scene as modified only if it received scene actions other than flush
            // also mark scene as modified if it had an active shader animation before (to not stop the animation with an empty flush)
            const bool isFlushWithChanges = !pendingFlush.sceneActions.empty() || pendingFlush.timeInfo.isEffectTimeSync || hadActiveShaderAnimation;
            if (isFlushWithChanges)
                // there are changes to scene -> mark it as modified to be re-rendered
                m_modifiedScenesToRerender.put(sceneID);
            else if (m_sceneStateExecutor.getSceneState(sceneID) == ESceneState::Rendered)
                // there are no changes to scene and it might not be rendered due to skipping of frames optimization,
                // mark it as if rendered for expiration monitor so that it does not expire
                m_expirationMonitor.onRendered(sceneID);
        }

        if (!pendingData.sceneReferenceActions.empty())
        {
            assert(m_sceneReferenceLogic);
            m_sceneReferenceLogic->addActions(sceneID, pendingData.sceneReferenceActions);
        }
    }

    void RendererSceneUpdater::processStagedResourceChangesFromAppliedFlushes()
    {
        // process resource changes only if there are no pending flushes
        for (const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            auto& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);
            auto& pendingData = stagingInfo.pendingData;

            if (pendingData.allPendingFlushesApplied)
            {
                // process staged resource changes only if ALL pending flushes were applied
                processStagedResourceChanges(sceneID, stagingInfo);
                PendingData::Clear(pendingData);
            }
        }
    }

    void RendererSceneUpdater::processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo)
    {
        // if scene is mapped unreference client resources that are no longer needed
        // and execute collected scene resource actions
        PendingData& pendingData = stagingInfo.pendingData;
        if (m_sceneStateExecutor.getSceneState(sceneID) >= ESceneState::MappingAndUploading)
        {
            assert(m_displayResourceManager);
            for (const auto& pendingFlush : pendingData.pendingFlushes)
                m_displayResourceManager->unreferenceResourcesForScene(sceneID, pendingFlush.resourcesRemoved);

            PendingSceneResourcesUtils::ApplySceneResourceActions(pendingData.sceneResourceActions, m_rendererScenes.getScene(sceneID), *m_displayResourceManager);
        }
    }

    void RendererSceneUpdater::handleECStreamAvailabilityChanges()
    {
        if (!m_renderer.hasDisplayController())
            return;

        IEmbeddedCompositingManager& embeddedCompositingManager = m_renderer.getDisplayController().getEmbeddedCompositingManager();
        if (embeddedCompositingManager.hasRealCompositor())
        {
            WaylandIviSurfaceIdVector streamsWithAvailabilityChanged;
            WaylandIviSurfaceIdVector newStreams;
            WaylandIviSurfaceIdVector obsoleteStreams;
            embeddedCompositingManager.dispatchStateChangesOfSources(streamsWithAvailabilityChanged, newStreams, obsoleteStreams);

            for (const auto stream : newStreams)
            {
                m_rendererEventCollector.addStreamSourceEvent(ERendererEventType::StreamSurfaceAvailable, stream);
            }
            for (const auto stream : obsoleteStreams)
            {
                m_rendererEventCollector.addStreamSourceEvent(ERendererEventType::StreamSurfaceUnavailable, stream);
                m_renderer.getStatistics().untrackStreamTexture(stream);
            }

            auto& texLinks = m_rendererScenes.getSceneLinksManager().getTextureLinkManager();
            StreamBufferLinkVector links;

            for (const auto changedStream : streamsWithAvailabilityChanged)
            {
                const auto& streamUsage = m_displayResourceManager->getStreamUsage(changedStream);
                // mark renderables using samplers with linked to stream buffers with changed availability as dirty
                for (const auto streamBuffer : streamUsage)
                {
                    links.clear();
                    texLinks.getStreamBufferLinks().getLinkedConsumers(streamBuffer, links);
                    for (const auto& link : links)
                    {
                        auto& scene = m_rendererScenes.getScene(link.consumerSceneId);
                        auto& dataSlot = scene.getDataSlot(link.consumerSlot);
                        scene.setRenderableResourcesDirtyByTextureSampler(dataSlot.attachedTextureSampler);
                        m_modifiedScenesToRerender.put(link.consumerSceneId);
                    }
                }
            }
        }
    }

    void RendererSceneUpdater::uploadAndUnloadVertexArrays()
    {
        for (const auto& sceneIt : m_rendererScenes)
        {
            const SceneId sceneId = sceneIt.key;
            if (m_sceneStateExecutor.getSceneState(sceneId) == ESceneState::Rendered)
            {
                RendererCachedScene& rendererScene = *(sceneIt.value.scene);

                if (rendererScene.hasDirtyVertexArrays())
                {
                    assert(m_displayResourceManager);
                    m_tempRenderablesWithUpdatedVertexArrays.clear();

                    bool dirtyVAOLeft = false;

                    const auto& vertexArraysDirtinessFlags = rendererScene.getVertexArraysDirtinessFlags();
                    for (RenderableHandle renderable(0u); renderable < rendererScene.getRenderableCount(); ++renderable)
                    {
                        if (vertexArraysDirtinessFlags[renderable.asMemoryHandle()])
                        {
                            bool updated = false;

                            // first delete current VAO
                            const auto& obsoleteVertexArray = rendererScene.getCachedHandlesForVertexArrays()[renderable.asMemoryHandle()];
                            if (obsoleteVertexArray.deviceHandle.isValid())
                            {
                                m_displayResourceManager->unloadVertexArray(renderable, sceneId);
                                updated = true;
                            }

                            // upload new VAO based on updated parameters
                            if (rendererScene.isRenderableAllocated(renderable))
                            {
                                if (!rendererScene.renderableResourcesDirty(renderable)
                                    && rendererScene.getRenderable(renderable).visibilityMode != EVisibilityMode::Off)
                                {
                                    SceneResourceUploader::UploadVertexArray(rendererScene, renderable, *m_displayResourceManager);
                                    updated = true;
                                }
                                else
                                    dirtyVAOLeft = true;
                            }

                            if (updated)
                                m_tempRenderablesWithUpdatedVertexArrays.push_back(renderable);
                        }
                    }

                    rendererScene.updateRenderableVertexArrays(*m_displayResourceManager, m_tempRenderablesWithUpdatedVertexArrays);
                    if (!dirtyVAOLeft)
                        rendererScene.markVertexArraysClean();
                }
            }
        }
    }

    void RendererSceneUpdater::updateScenesResourceCache()
    {
        // update renderer scenes renderables and resource cache
        for (const auto& sceneIt : m_rendererScenes)
        {
            const SceneId sceneId = sceneIt.key;
            // update resource cache only if scene is actually rendered
            if (m_sceneStateExecutor.getSceneState(sceneId) == ESceneState::Rendered)
            {
                RendererCachedScene& rendererScene = *(sceneIt.value.scene);
                rendererScene.updateRenderablesAndResourceCache(*m_displayResourceManager);
            }
        }
    }

    void RendererSceneUpdater::updateScenesStates()
    {
        SceneIdVector scenesMapped;
        SceneIdVector scenesToForceUnsubscribe;
        for (const auto& it : m_scenesToBeMapped)
        {
            const SceneId sceneId = it.first;
            const ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneId);

            switch (sceneState)
            {
            case ESceneState::MapRequested:
            {
                assert(m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes.empty());
                const IDisplayController& displayController = m_renderer.getDisplayController();
                m_renderer.assignSceneToDisplayBuffer(sceneId, displayController.getDisplayBuffer(), 0);
                m_sceneStateExecutor.setMappingAndUploading(sceneId);
                // mapping a scene needs re-request of all its resources at the new resource manager
                if (!markClientAndSceneResourcesForReupload(sceneId))
                {
                    LOG_ERROR(CONTEXT_RENDERER, "Failed to upload all scene resources within time budget (" << m_frameTimer.getTimeBudgetForSection(EFrameTimerSectionBudget::SceneResourcesUpload).count() << " us)."
                        << " Reduce amount of scene resources or use client resources instead! Scene " << sceneId << " will be force unsubscribed!");
                    scenesToForceUnsubscribe.push_back(sceneId);
                }
            }
                break;
            case ESceneState::MappingAndUploading:
            {
                const IRendererResourceManager& resourceManager = *m_displayResourceManager;
                const StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneId);

                bool canBeMapped = false;
                // allow map only if there are no pending flushes and all used resources uploaded
                if (stagingInfo.pendingData.pendingFlushes.empty())
                {
                    const auto usedResources = resourceManager.getResourcesInUseByScene(sceneId);
                    canBeMapped = usedResources == nullptr || std::all_of(usedResources->cbegin(), usedResources->cend(),
                        [&](const auto& res) { return resourceManager.getResourceStatus(res) == EResourceStatus::Uploaded; });
                }

                if (!canBeMapped)
                    canBeMapped = checkIfForceMapNeeded(sceneId);

                if (canBeMapped)
                {
                    m_sceneStateExecutor.setMapped(sceneId);
                    scenesMapped.push_back(sceneId);
                    // force retrigger all render once passes,
                    // if scene was rendered before and is remapped, render once passes need to be rendered again
                    m_rendererScenes.getScene(sceneId).retriggerAllRenderOncePasses();
                }
            }
                break;
            default:
                assert(false);
                break;
            }
        }

        for (const auto sceneId : scenesToForceUnsubscribe)
            handleSceneUnsubscriptionRequest(sceneId, true);

        for (const auto sceneId : scenesMapped)
        {
            m_scenesToBeMapped.erase(sceneId);

            const auto& pendingFlushes = m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes;
            if (!pendingFlushes.empty())
            {
                LOG_ERROR(CONTEXT_RENDERER, "Scene " << sceneId << " - expected no pending flushes at this point");
                assert(pendingFlushes.size() > m_maximumPendingFlushes);
            }
        }

        for (const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneId = rendererScene.key;
            if (m_sceneStateExecutor.getSceneState(sceneId) == ESceneState::RenderRequested)
            {
                m_renderer.resetRenderInterruptState();
                m_renderer.setSceneShown(sceneId, true);
                m_sceneStateExecutor.setRendered(sceneId);
                // in case there are any scenes depending on this scene via OB link,
                // mark it as modified so that OB link dependency checker re-renders all that need it
                m_modifiedScenesToRerender.put(sceneId);
            }
        }
    }

    bool RendererSceneUpdater::checkIfForceMapNeeded(SceneId sceneId)
    {
        constexpr std::chrono::seconds MappingLogPeriod{ 1u };
        constexpr size_t MaxNumResourcesToLog = 20u;

        assert(m_scenesToBeMapped.count(sceneId) != 0);
        auto& mapRequest = m_scenesToBeMapped[sceneId];
        const auto currentFrameTime = m_frameTimer.getFrameStartTime();
        const auto totalWaitingTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - mapRequest.requestTimeStamp);

        // log missing resources every period
        if (currentFrameTime - mapRequest.lastLogTimeStamp > MappingLogPeriod)
        {
            LOG_WARN_F(CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& logger)
            {
                logger << "Scene " << sceneId << " waiting " << totalWaitingTime.count() << " ms for resources in order to be mapped: ";
                size_t numResourcesWaiting = 0u;
                const auto usedResources = m_displayResourceManager->getResourcesInUseByScene(sceneId);
                if (usedResources)
                {
                    for (const auto& res : *usedResources)
                    {
                        const auto resStatus = m_displayResourceManager->getResourceStatus(res);
                        if (resStatus != EResourceStatus::Uploaded)
                        {
                            if (++numResourcesWaiting <= MaxNumResourcesToLog)
                                logger << res << " <" << resStatus << ">; ";
                        }
                    }
                }
                logger << " " << numResourcesWaiting << " unresolved resources in total";
            });

            mapRequest.lastLogTimeStamp = currentFrameTime;
        }

        // check if force map needed due to too many pending flushes
        const auto numPendingFlushes = getNumberOfPendingNonEmptyFlushes(sceneId);
        if (numPendingFlushes > m_maximumPendingFlushes)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Force mapping scene " << sceneId << " due to " << numPendingFlushes << " pending flushes, renderer cannot catch up with resource updates.");
            const auto usedResources = m_displayResourceManager->getResourcesInUseByScene(sceneId);
            if (usedResources)
                logMissingResources(*usedResources, sceneId);

            return true;
        }

        // check if force map needed due to too long waiting
        if (totalWaitingTime > m_maximumWaitingTimeToForceMap)
        {
            // Scene might already have broken resources in it before mapping and then new flushes won't be blocked
            // therefore this additional time-based criteria for force mapping.
            // It is still error but it is not necessary to block on it forever.
            LOG_ERROR(CONTEXT_RENDERER, "Force mapping scene " << sceneId << " due to " << totalWaitingTime.count()
                << " ms waiting time to be mapped, renderer cannot catch up with resource updates or scene has broken resources.");
            return true;
        }

        return false;
    }

    void RendererSceneUpdater::applySceneActions(RendererCachedScene& scene, PendingFlush& flushInfo)
    {
        const SceneActionCollection& actionsForScene = flushInfo.sceneActions;
        const UInt32 numActions = actionsForScene.numberOfActions();
        LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::applySceneActions start applying scene actions [count:" << numActions << "] for scene with id " << scene.getSceneId());

        SceneActionApplier::ApplyActionsOnScene(scene, actionsForScene);

        LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::applySceneActions finished applying scene actions for scene with id " << scene.getSceneId());
    }

    void RendererSceneUpdater::destroyScene(SceneId sceneID)
    {
        m_renderer.resetRenderInterruptState();
        const ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneID);
        switch (sceneState)
        {
        case ESceneState::Rendered:
            m_renderer.setSceneShown(sceneID, false);
            RFALLTHROUGH;
        case ESceneState::RenderRequested:
        case ESceneState::Mapped:
        case ESceneState::MappingAndUploading:
            unloadSceneResourcesAndUnrefSceneResources(sceneID);
            m_renderer.unassignScene(sceneID);
            RFALLTHROUGH;
        case ESceneState::MapRequested:
        case ESceneState::Subscribed:
        case ESceneState::SubscriptionPending:
            m_rendererScenes.destroyScene(sceneID);
            m_renderer.getStatistics().untrackScene(sceneID);
            RFALLTHROUGH;
        default:
            break;
        }

        if (m_scenesToBeMapped.count(sceneID) != 0)
        {
            assert(sceneState == ESceneState::MapRequested || sceneState == ESceneState::MappingAndUploading);
            m_scenesToBeMapped.erase(sceneID);
        }

        m_expirationMonitor.onDestroyed(sceneID);
    }

    void RendererSceneUpdater::unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        assert(SceneStateIsAtLeast(m_sceneStateExecutor.getSceneState(sceneId), ESceneState::MappingAndUploading));
        IRendererResourceManager& resourceManager = *m_displayResourceManager;

        resourceManager.unloadAllSceneResourcesForScene(sceneId);
        resourceManager.unreferenceAllResourcesForScene(sceneId);

        RendererCachedScene& rendererScene = m_rendererScenes.getScene(sceneId);
        rendererScene.resetResourceCache();
    }

    bool RendererSceneUpdater::markClientAndSceneResourcesForReupload(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        assert(ESceneState::MappingAndUploading == m_sceneStateExecutor.getSceneState(sceneId));
        IRendererResourceManager& resourceManager = *m_displayResourceManager;

        // collect all scene resources in scene and upload them
        const auto& scene = m_rendererScenes.getScene(sceneId);
        SceneResourceActionVector sceneResourceActions;
        size_t sceneResourcesByteSize = 0u;
        ResourceUtils::GetAllSceneResourcesFromScene(sceneResourceActions, scene, sceneResourcesByteSize);
        if (!sceneResourceActions.empty())
        {
            if (sceneResourcesByteSize > 0u)
                LOG_INFO(CONTEXT_RENDERER, "Applying scene resources gathered from scene " << sceneId << ", " << sceneResourceActions.size() << " actions, " << sceneResourcesByteSize << " bytes");

            // enable time measuring and interrupting of upload only if scene is remote
            const bool sceneIsRemote = (m_sceneStateExecutor.getScenePublicationMode(sceneId) != EScenePublicationMode_LocalOnly);
            if (!PendingSceneResourcesUtils::ApplySceneResourceActions(sceneResourceActions, scene, resourceManager, sceneIsRemote ? &m_frameTimer : nullptr))
                return false;
        }

        // reference all the resources in use by the scene to be mapped
        ResourceContentHashVector resourcesUsedInScene;
        ResourceUtils::GetAllResourcesFromScene(resourcesUsedInScene, scene);
        if (!resourcesUsedInScene.empty())
        {
            LOG_INFO(CONTEXT_RENDERER, "Marking " << resourcesUsedInScene.size() << " client resources as used by scene " << sceneId);

            // compare actual 'in-use' list with 'to be uploaded for mapping' list
            // the only case these do not contain same elements is re-map case (other than first mapping)
            const auto& providedResources = m_rendererScenes.getStagingInfo(sceneId).resourcesToUploadOnceMapping;
            ResourceContentHashVector providedHashes{ providedResources.size() };
            std::transform(providedResources.cbegin(), providedResources.cend(), providedHashes.begin(), [](const auto& mr) { return mr->getHash(); });

            std::sort(resourcesUsedInScene.begin(), resourcesUsedInScene.end());
            std::sort(providedHashes.begin(), providedHashes.end());
            if (resourcesUsedInScene != providedHashes)
            {
                // For mapping which is not first new logic does not trigger reference of those resources (it only works with incoming flushes),
                // therefore here we explicitly reference them for new logic - in a way we simulate they came in flushes.
                // However there might be some resources (with data) which already came in flushes between mapping, those are stored in 'to be uploaded for mapping' list,
                // those will be referenced and provided by new logic automatically next frame. So we reference only resources in use that are not waiting provided.
                ResourceContentHashVector resourcesToReference;
                std::set_difference(resourcesUsedInScene.cbegin(), resourcesUsedInScene.cend(), providedHashes.cbegin(), providedHashes.cend(), std::back_inserter(resourcesToReference));
                resourceManager.referenceResourcesForScene(sceneId, resourcesToReference);
                LOG_INFO_P(CONTEXT_RENDERER, "Out of {} resources used in scene {} there are {} resources that are not ready to be provided and uploaded unless already cached.", resourcesUsedInScene.size(), sceneId, resourcesToReference.size());
            }
            else
                LOG_INFO_P(CONTEXT_RENDERER, "All resources used in scene {} are ready to be provided and uploaded.", sceneId);
        }

        return true;
    }

    void RendererSceneUpdater::handleScenePublished(SceneId sceneId, EScenePublicationMode mode)
    {
        if (m_sceneStateExecutor.checkIfCanBePublished(sceneId))
        {
            assert(!m_rendererScenes.hasScene(sceneId));
            m_sceneStateExecutor.setPublished(sceneId, mode);
        }
    }

    void RendererSceneUpdater::handleSceneUnpublished(SceneId sceneId)
    {
        if (m_sceneStateExecutor.checkIfCanBeUnpublished(sceneId))
        {
            destroyScene(sceneId);
            m_sceneStateExecutor.setUnpublished(sceneId);
        }
    }

    void RendererSceneUpdater::handleSceneSubscriptionRequest(SceneId sceneId)
    {
        if (m_sceneStateExecutor.checkIfCanBeSubscriptionRequested(sceneId))
        {
            assert(!m_rendererScenes.hasScene(sceneId));
            m_sceneStateExecutor.setSubscriptionRequested(sceneId);
        }
    }

    void RendererSceneUpdater::handleSceneUnsubscriptionRequest(SceneId sceneId, bool indirect)
    {
        if (!indirect && !m_sceneStateExecutor.checkIfCanBeUnsubscribed(sceneId))
        {
            return;
        }

        destroyScene(sceneId);
        assert(!m_rendererScenes.hasScene(sceneId));
        m_sceneStateExecutor.setUnsubscribed(sceneId, indirect);
    }

    void RendererSceneUpdater::handleSceneMappingRequest(SceneId sceneId)
    {
        if (m_sceneStateExecutor.checkIfCanBeMapRequested(sceneId))
        {
            m_sceneStateExecutor.setMapRequested(sceneId);
            assert(m_scenesToBeMapped.count(sceneId) == 0);
            m_scenesToBeMapped.insert({ sceneId, { m_frameTimer.getFrameStartTime(), m_frameTimer.getFrameStartTime() } });
        }
    }

    void RendererSceneUpdater::handleSceneUnmappingRequest(SceneId sceneId)
    {
        if (m_sceneStateExecutor.checkIfCanBeUnmapped(sceneId))
        {
            const auto sceneState = m_sceneStateExecutor.getSceneState(sceneId);
            if (sceneState == ESceneState::MappingAndUploading || sceneState == ESceneState::MapRequested)
            {
                // scene unmap requested before reaching mapped state (cancel mapping), emit map failed event
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneMapFailed, sceneId);
                m_scenesToBeMapped.erase(sceneId);
            }

            switch (sceneState)
            {
            case ESceneState::Mapped:
                m_rendererScenes.getSceneLinksManager().handleSceneUnmapped(sceneId);
                RFALLTHROUGH;
            case ESceneState::MappingAndUploading:
                // scene was already internally mapped and needs unload/unreference of all its resources from its resource manager
                unloadSceneResourcesAndUnrefSceneResources(sceneId);
                m_renderer.unassignScene(sceneId);
                RFALLTHROUGH;
            case ESceneState::MapRequested:
                m_sceneStateExecutor.setUnmapped(sceneId);
                break;
            default:
                assert(false);
            }
        }
    }

    void RendererSceneUpdater::handleSceneShowRequest(SceneId sceneId)
    {
        if (m_sceneStateExecutor.checkIfCanBeRenderedRequested(sceneId))
        {
            assert(m_rendererScenes.hasScene(sceneId));
            m_sceneStateExecutor.setRenderedRequested(sceneId);
        }
    }

    void RendererSceneUpdater::handleSceneHideRequest(SceneId sceneId)
    {
        const auto sceneState = m_sceneStateExecutor.getSceneState(sceneId);
        if (sceneState == ESceneState::RenderRequested)
        {
            // this essentially cancels the previous (not yet executed) show command
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType::SceneShowFailed, sceneId);
            m_sceneStateExecutor.setHidden(sceneId);
        }
        else if (m_sceneStateExecutor.checkIfCanBeHidden(sceneId))
        {
            assert(m_rendererScenes.hasScene(sceneId));
            m_renderer.resetRenderInterruptState();
            m_renderer.setSceneShown(sceneId, false);
            m_sceneStateExecutor.setHidden(sceneId);
            m_expirationMonitor.onHidden(sceneId);
        }
    }

    void RendererSceneUpdater::handleSceneReceived(const SceneInfo& sceneInfo)
    {
        if (m_sceneStateExecutor.checkIfCanBeSubscriptionPending(sceneInfo.sceneID))
        {
            m_rendererScenes.createScene(sceneInfo);
            m_sceneStateExecutor.setSubscriptionPending(sceneInfo.sceneID);
        }
    }

    bool RendererSceneUpdater::handleBufferCreateRequest(OffscreenBufferHandle buffer, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType)
    {
        bool success = false;
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferCreateRequest cannot create an offscreen buffer on invalid display.");
        }
        else
        {
            assert(m_displayResourceManager);
            IRendererResourceManager& resourceManager = *m_displayResourceManager;
            if (resourceManager.getOffscreenBufferDeviceHandle(buffer).isValid())
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferCreateRequest an offscreen buffer with the same ID (" << buffer << ") already exists!");
            }
            else
            {
                resourceManager.uploadOffscreenBuffer(buffer, width, height, sampleCount, isDoubleBuffered, depthStencilBufferType);
                const DeviceResourceHandle deviceHandle = resourceManager.getOffscreenBufferDeviceHandle(buffer);
                m_renderer.resetRenderInterruptState();
                m_renderer.registerOffscreenBuffer(deviceHandle, width, height, isDoubleBuffered);

                LOG_INFO_P(CONTEXT_RENDERER, "Created offscreen buffer {} (device handle {}): {}x{} {}", buffer, deviceHandle, width, height, (isDoubleBuffered ? " interruptible" : ""));
                success = true;
            }
        }

        m_rendererEventCollector.addOBEvent((success ? ERendererEventType::OffscreenBufferCreated : ERendererEventType::OffscreenBufferCreateFailed), buffer, m_display, -1, 0);

        return success;
    }

    bool RendererSceneUpdater::handleDmaBufferCreateRequest(OffscreenBufferHandle buffer, UInt32 width, UInt32 height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers)
    {
        bool success = false;
        int dmaBufferFD = -1;
        UInt32 dmaBufferStride = 0u;
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleDmaBufferCreateRequest cannot create an offscreen buffer on invalid display.");
        }
        else
        {
            assert(m_displayResourceManager);
            IRendererResourceManager& resourceManager = *m_displayResourceManager;
            if (resourceManager.getOffscreenBufferDeviceHandle(buffer).isValid())
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleDmaBufferCreateRequest an offscreen buffer with the same ID (" << buffer << ") already exists!");
            }
            else
            {
                resourceManager.uploadDmaOffscreenBuffer(buffer, width, height, dmaBufferFourccFormat, dmaBufferUsageFlags, dmaBufferModifiers);
                const DeviceResourceHandle deviceHandle = resourceManager.getOffscreenBufferDeviceHandle(buffer);
                m_renderer.resetRenderInterruptState();
                m_renderer.registerOffscreenBuffer(deviceHandle, width, height, false);

                LOG_INFO_P(CONTEXT_RENDERER, "Created DMA offscreen buffer {} (device handle {}): {}x{}", buffer, deviceHandle, width, height);
                dmaBufferFD = resourceManager.getDmaOffscreenBufferFD(buffer);
                dmaBufferStride = resourceManager.getDmaOffscreenBufferStride(buffer);
                success = true;
            }
        }

        m_rendererEventCollector.addOBEvent((success ? ERendererEventType::OffscreenBufferCreated : ERendererEventType::OffscreenBufferCreateFailed), buffer, m_display, dmaBufferFD, dmaBufferStride);

        return success;
    }

    bool RendererSceneUpdater::handleBufferDestroyRequest(OffscreenBufferHandle buffer)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest cannot destroy an offscreen buffer on invalid display.");
            return false;
        }

        assert(m_displayResourceManager);
        IRendererResourceManager& resourceManager = *m_displayResourceManager;
        const auto bufferDeviceHandle = resourceManager.getOffscreenBufferDeviceHandle(buffer);
        if (!bufferDeviceHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest could not find buffer with ID " << buffer);
            return false;
        }

        for (const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneId = rendererScene.key;
            const auto sceneDisplayBuffer = m_renderer.getBufferSceneIsAssignedTo(sceneId);
            if (sceneDisplayBuffer == bufferDeviceHandle)
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest cannot destroy buffer " << buffer << ", there is one or more scenes assigned to it, unmap or reassign them first.");
                return false;
            }
        }

        m_rendererScenes.getSceneLinksManager().handleBufferDestroyed(buffer);
        m_renderer.resetRenderInterruptState();
        m_renderer.unregisterOffscreenBuffer(bufferDeviceHandle);
        resourceManager.unloadOffscreenBuffer(buffer);

        return true;
    }

    bool RendererSceneUpdater::handleExternalBufferCreateRequest(ExternalBufferHandle buffer)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleExternalBufferCreateRequest cannot create external buffer on invalid display.");
        }
        else
        {
            assert(m_displayResourceManager);
            IRendererResourceManager& resourceManager = *m_displayResourceManager;
            resourceManager.uploadExternalBuffer(buffer);

            const DeviceResourceHandle deviceHandle = resourceManager.getExternalBufferDeviceHandle(buffer);
            if (deviceHandle.isValid())
            {
                const uint32_t glTexId = resourceManager.getExternalBufferGlId(buffer);
                LOG_INFO_P(CONTEXT_RENDERER, "Created external buffer {} (device handle {}, GL id: {})", buffer, deviceHandle, glTexId);

                m_rendererEventCollector.addExternalBufferEvent(ERendererEventType::ExternalBufferCreated, m_display, buffer, glTexId);
                return true;
            }
        }

        LOG_ERROR_P(CONTEXT_RENDERER, "Failed creating external buffer {}", buffer);

        m_rendererEventCollector.addExternalBufferEvent(ERendererEventType::ExternalBufferCreateFailed, m_display, buffer, 0u);
        return false;
    }

    bool RendererSceneUpdater::handleExternalBufferDestroyRequest(ExternalBufferHandle buffer)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleExternalBufferDestroyRequest cannot destroy an external buffer on invalid display.");
        }
        else
        {
            assert(m_displayResourceManager);
            IRendererResourceManager& resourceManager = *m_displayResourceManager;
            const auto bufferDeviceHandle = resourceManager.getExternalBufferDeviceHandle(buffer);
            if (!bufferDeviceHandle.isValid())
            {
                LOG_ERROR_P(CONTEXT_RENDERER, "RendererSceneUpdater::handleExternalBufferDestroyRequest could not find external buffer {}", buffer);
            }
            else
            {
                m_rendererScenes.getSceneLinksManager().handleBufferDestroyed(buffer);

                resourceManager.unloadExternalBuffer(buffer);

                m_rendererEventCollector.addExternalBufferEvent(ERendererEventType::ExternalBufferDestroyed, m_display, buffer, 0u);
                return true;
            }
        }

        m_rendererEventCollector.addExternalBufferEvent(ERendererEventType::ExternalBufferDestroyFailed, m_display, buffer, 0u);
        return false;
    }

    bool RendererSceneUpdater::handleBufferCreateRequest(StreamBufferHandle buffer, WaylandIviSurfaceId source)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferCreateRequest cannot create a stream buffer on invalid display.");
            return false;
        }

        assert(m_displayResourceManager);
        IRendererResourceManager& resourceManager = *m_displayResourceManager;
        resourceManager.uploadStreamBuffer(buffer, source);

        const DeviceResourceHandle deviceHandle = resourceManager.getStreamBufferDeviceHandle(buffer);
        LOG_INFO(CONTEXT_RENDERER, "Created stream buffer " << buffer << " (device handle " << deviceHandle << ")");

        return true;
    }

    bool RendererSceneUpdater::handleBufferDestroyRequest(StreamBufferHandle buffer)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest cannot destroy a stream buffer on invalid display.");
            return false;
        }

        assert(m_displayResourceManager);
        IRendererResourceManager& resourceManager = *m_displayResourceManager;
        resourceManager.unloadStreamBuffer(buffer);
        m_rendererScenes.getSceneLinksManager().handleBufferDestroyedOrSourceUnavailable(buffer);

        return true;
    }

    void RendererSceneUpdater::handleSetClearFlags(OffscreenBufferHandle buffer, uint32_t clearFlags)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearFlags cannot set clear flags on invalid display.");
            return;
        }

        DeviceResourceHandle bufferDeviceHandle;
        if (buffer.isValid())
        {
            bufferDeviceHandle = m_displayResourceManager->getOffscreenBufferDeviceHandle(buffer);
            if (!bufferDeviceHandle.isValid())
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearEnabled cannot enable/disable clear for unknown offscreen buffer " << buffer);
                return;
            }
        }
        else
            bufferDeviceHandle = m_renderer.getDisplayController().getDisplayBuffer();

        assert(bufferDeviceHandle.isValid());
        m_renderer.setClearFlags(bufferDeviceHandle, clearFlags);
    }

    void RendererSceneUpdater::handleSetClearColor(OffscreenBufferHandle buffer, const Vector4& clearColor)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearColor cannot set clear color on invalid display.");
            return;
        }

        DeviceResourceHandle bufferDeviceHandle;
        if (buffer.isValid())
        {
            bufferDeviceHandle = m_displayResourceManager->getOffscreenBufferDeviceHandle(buffer);
            if (!bufferDeviceHandle.isValid())
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearColor cannot set clear color for unknown offscreen buffer " << buffer);
                return;
            }
        }
        else
            bufferDeviceHandle = m_renderer.getDisplayController().getDisplayBuffer();

        assert(bufferDeviceHandle.isValid());
        m_renderer.setClearColor(bufferDeviceHandle, clearColor);
    }

    void RendererSceneUpdater::handleSetExternallyOwnedWindowSize(uint32_t width, uint32_t height)
    {
        if (!m_renderer.hasDisplayController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetExternallyOwnedWindowSize cannot set window size on invalid display.");
            return;
        }

        if(!m_renderer.setExternallyOwnedWindowSize(width, height))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetExternallyOwnedWindowSize failed, platform does not support external window ownership or window not externally owned!");
        }
    }

    void RendererSceneUpdater::handleReadPixels(OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo)
    {
        bool readPixelsFailed = false;
        DeviceResourceHandle renderTargetHandle{};
        if (m_renderer.hasDisplayController())
        {
            const auto& displayResourceManager = *m_displayResourceManager;
            const auto& displayController = m_renderer.getDisplayController();

            if (buffer.isValid())
                renderTargetHandle = displayResourceManager.getOffscreenBufferDeviceHandle(buffer);
            else
                renderTargetHandle = displayController.getDisplayBuffer();

            if (renderTargetHandle.isValid())
            {
                const auto& bufferViewport = m_renderer.getDisplaySetup().getDisplayBuffer(renderTargetHandle).viewport;
                if (screenshotInfo.fullScreen)
                    screenshotInfo.rectangle = { 0u, 0u, bufferViewport.width, bufferViewport.height };
                else if (screenshotInfo.rectangle.x + screenshotInfo.rectangle.width > bufferViewport.width
                    || screenshotInfo.rectangle.y + screenshotInfo.rectangle.height > bufferViewport.height)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::readPixels failed, requested area is out of offscreen display/buffer size boundaries!");
                    readPixelsFailed = true;
                }
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::readPixels failed, requested buffer does not exist : " << buffer << " !");
                readPixelsFailed = true;
            }
        }

        if (readPixelsFailed)
        {
            if (screenshotInfo.filename.empty())
                // only generate event when not saving pixels to file!
                m_rendererEventCollector.addReadPixelsEvent(ERendererEventType::ReadPixelsFromFramebufferFailed, m_display, buffer, {});
        }
        else
            m_renderer.scheduleScreenshot(renderTargetHandle, std::move(screenshotInfo));
    }

    Bool RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder)
    {
        if (!m_renderer.hasDisplayController() || !m_rendererScenes.hasScene(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest cannot assign scene " << sceneId << " to a display buffer, display invalid or scene is not mapped.");
            return false;
        }

        assert(m_displayResourceManager);
        const IRendererResourceManager& resourceManager = *m_displayResourceManager;
        // determine if assigning to display's framebuffer or an offscreen buffer
        const DeviceResourceHandle bufferDeviceHandle = (buffer.isValid() ? resourceManager.getOffscreenBufferDeviceHandle(buffer) : m_renderer.getDisplayController().getDisplayBuffer());
        if (!bufferDeviceHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest cannot assign scene to render buffer, buffer " << buffer << " not found on this display.");
            return false;
        }

        m_renderer.resetRenderInterruptState();
        m_renderer.assignSceneToDisplayBuffer(sceneId, bufferDeviceHandle, sceneRenderOrder);

        return true;
    }

    void RendererSceneUpdater::handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (m_rendererScenes.hasScene(providerSceneId) && m_rendererScenes.hasScene(consumerSceneId))
        {
            const DataSlotHandle providerSlotHandle = DataLinkUtils::GetDataSlotHandle(providerSceneId, providerId, m_rendererScenes);
            const DataSlotHandle consumerSlotHandle = DataLinkUtils::GetDataSlotHandle(consumerSceneId, consumerId, m_rendererScenes);

            if (providerSlotHandle.isValid() && consumerSlotHandle.isValid())
            {
                const EDataSlotType providerSlotType = DataLinkUtils::GetDataSlot(providerSceneId, providerSlotHandle, m_rendererScenes).type;
                if (providerSlotType == EDataSlotType_TextureProvider)
                {
                    if (m_sceneStateExecutor.getSceneState(providerSceneId) < ESceneState::Mapped)
                    {
                        LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: provider scene providing a texture must be fully mapped before linking! (Provider scene: " << providerSceneId << ") (Consumer scene: " << consumerSceneId << ")");
                        m_rendererEventCollector.addDataLinkEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
                        return;
                    }
                }
            }
        }

        m_rendererScenes.getSceneLinksManager().createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
        m_modifiedScenesToRerender.put(consumerSceneId);
        m_renderer.resetRenderInterruptState();
    }

    void RendererSceneUpdater::handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_renderer.hasDisplayController() || !m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Link offscreen buffer to consumer scene " << consumerSceneId << " failed, invalid display or scene not mapped.");
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        assert(m_displayResourceManager);
        const IRendererResourceManager& resourceManager = *m_displayResourceManager;
        if (!resourceManager.getOffscreenBufferDeviceHandle(buffer).isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Link offscreen buffer failed: offscreen buffer " << buffer << " has to exist on the same display where the consumer scene " << consumerSceneId << " is mapped!");
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        m_rendererScenes.getSceneLinksManager().createBufferLink(buffer, consumerSceneId, consumerId);
        m_modifiedScenesToRerender.put(consumerSceneId);
        m_renderer.resetRenderInterruptState();
    }

    void RendererSceneUpdater::handleBufferToSceneDataLinkRequest(StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_renderer.hasDisplayController() || !m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Link stream buffer to consumer scene " << consumerSceneId << " failed, invalid display or scene not mapped.");
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        m_rendererScenes.getSceneLinksManager().createBufferLink(buffer, consumerSceneId, consumerId);
        m_modifiedScenesToRerender.put(consumerSceneId);
        m_renderer.resetRenderInterruptState();
    }

    void RendererSceneUpdater::handleBufferToSceneDataLinkRequest(ExternalBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId)
    {
        if (!m_renderer.hasDisplayController() || !m_rendererScenes.hasScene(consumerSceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Link external buffer to consumer scene " << consumerSceneId << " failed, invalid display or scene not mapped.");
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        assert(m_displayResourceManager);
        const IRendererResourceManager& resourceManager = *m_displayResourceManager;
        if (!resourceManager.getExternalBufferDeviceHandle(buffer).isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Link external buffer failed: external buffer " << buffer << " has to exist on the same display where the consumer scene " << consumerSceneId << " is mapped!");
            m_rendererEventCollector.addBufferEvent(ERendererEventType::SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        m_rendererScenes.getSceneLinksManager().createBufferLink(buffer, consumerSceneId, consumerId);
        m_modifiedScenesToRerender.put(consumerSceneId);
        m_renderer.resetRenderInterruptState();
    }

    void RendererSceneUpdater::handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId)
    {
        m_rendererScenes.getSceneLinksManager().removeDataLink(consumerSceneId, consumerId);
        m_modifiedScenesToRerender.put(consumerSceneId);
        m_renderer.resetRenderInterruptState();
    }

    void RendererSceneUpdater::handlePickEvent(SceneId sceneId, Vector2 coordsNormalizedToBufferSize)
    {
        const auto bufferHandle = m_renderer.getBufferSceneIsAssignedTo(sceneId);
        if (!m_rendererScenes.hasScene(sceneId) || !bufferHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handlePickEvent could not process pick event for scene " << sceneId << " which is not assigned to any display buffer.");
            return;
        }

        const auto& buffer = m_renderer.getDisplaySetup().getDisplayBuffer(bufferHandle);
        const Vector2i coordsInBufferSpace = { static_cast<Int32>(std::lroundf((coordsNormalizedToBufferSize.x + 1.f) * buffer.viewport.width / 2.f)) ,
                                                static_cast<Int32>(std::lroundf((coordsNormalizedToBufferSize.y + 1.f) * buffer.viewport.height / 2.f)) };

        PickableObjectIds pickedObjects;
        const TransformationLinkCachedScene& scene = m_rendererScenes.getScene(sceneId);


        IntersectionUtils::CheckSceneForIntersectedPickableObjects(scene, coordsInBufferSpace, pickedObjects);
        if (!pickedObjects.empty())
            m_rendererEventCollector.addPickedEvent(ERendererEventType::ObjectsPicked, sceneId, std::move(pickedObjects));
    }

    Bool RendererSceneUpdater::hasPendingFlushes(SceneId sceneId) const
    {
        return m_rendererScenes.hasScene(sceneId) && !m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes.empty();
    }

    void RendererSceneUpdater::setLimitFlushesForceApply(UInt limitForPendingFlushesForceApply)
    {
        m_maximumPendingFlushes = limitForPendingFlushesForceApply;
    }

    void RendererSceneUpdater::setLimitFlushesForceUnsubscribe(UInt limitForPendingFlushesForceUnsubscribe)
    {
        m_maximumPendingFlushesToKillScene = limitForPendingFlushesForceUnsubscribe;
    }

    void RendererSceneUpdater::logRendererInfo(ERendererLogTopic topic, bool verbose, NodeHandle nodeFilter) const
    {
        RendererLogger::LogTopic(*this, topic, verbose, nodeFilter);
    }

    void RendererSceneUpdater::setSkippingOfUnmodifiedScenes(bool enable)
    {
        m_skipUnmodifiedScenes = enable;
    }

    void RendererSceneUpdater::setSceneReferenceLogicHandler(ISceneReferenceLogic& sceneRefLogic)
    {
        assert(m_sceneReferenceLogic == nullptr);
        m_sceneReferenceLogic = &sceneRefLogic;
    }

    bool RendererSceneUpdater::areResourcesFromPendingFlushesUploaded(SceneId sceneId) const
    {
        const auto& pendingData = m_rendererScenes.getStagingInfo(sceneId).pendingData;
        for (const auto& pendingFlush : pendingData.pendingFlushes)
            for (const auto& res : pendingFlush.resourcesAdded)
                if (m_displayResourceManager->getResourceStatus(res) != EResourceStatus::Uploaded)
                    return false;

        return true;
    }

    void RendererSceneUpdater::updateScenesShaderAnimations()
    {
        for (const auto& scene : m_rendererScenes)
        {
            const SceneId sceneID = scene.key;
            if (m_sceneStateExecutor.getSceneState(sceneID) == ESceneState::Rendered)
            {
                if (scene.value.scene->hasActiveShaderAnimation())
                    m_modifiedScenesToRerender.put(sceneID);
            }
        }
    }

    void RendererSceneUpdater::updateScenesTransformationCache()
    {
        m_scenesNeedingTransformationCacheUpdate.clear();
        for(const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            if (m_sceneStateExecutor.getSceneState(sceneID) == ESceneState::Rendered)
            {
                m_scenesNeedingTransformationCacheUpdate.put(sceneID);
            }
        }

        const SceneIdVector& dependencyOrderedScenes = m_rendererScenes.getSceneLinksManager().getTransformationLinkManager().getDependencyChecker().getDependentScenesInOrder();
        for(const auto sceneId : dependencyOrderedScenes)
        {
            if (m_scenesNeedingTransformationCacheUpdate.contains(sceneId))
            {
                RendererCachedScene& renderScene = m_rendererScenes.getScene(sceneId);
                renderScene.updateRenderableWorldMatricesWithLinks();
                m_scenesNeedingTransformationCacheUpdate.remove(sceneId);
            }
        }

        // update rest of scenes that have no dependencies
        for(const auto sceneId : m_scenesNeedingTransformationCacheUpdate)
        {
            RendererCachedScene& renderScene = m_rendererScenes.getScene(sceneId);
            renderScene.updateRenderableWorldMatrices();
        }
    }

    void RendererSceneUpdater::updateScenesDataLinks()
    {
        const auto& dataRefLinkManager = m_rendererScenes.getSceneLinksManager().getDataReferenceLinkManager();
        const auto& transfLinkManager = m_rendererScenes.getSceneLinksManager().getTransformationLinkManager();
        const auto& texLinkManager = m_rendererScenes.getSceneLinksManager().getTextureLinkManager();

        resolveDataLinksForConsumerScenes(dataRefLinkManager);

        markScenesDependantOnModifiedConsumersAsModified(dataRefLinkManager, transfLinkManager, texLinkManager);
        markScenesDependantOnModifiedOffscreenBuffersAsModified(texLinkManager);
    }

    void RendererSceneUpdater::resolveDataLinksForConsumerScenes(const DataReferenceLinkManager& dataRefLinkManager)
    {
        for(const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            if (dataRefLinkManager.getDependencyChecker().hasDependencyAsConsumer(sceneID))
            {
                if (m_sceneStateExecutor.getSceneState(sceneID) == ESceneState::Rendered)
                {
                    DataReferenceLinkCachedScene& scene = m_rendererScenes.getScene(sceneID);
                    dataRefLinkManager.resolveLinksForConsumerScene(scene);
                }
            }
        }
    }
    void RendererSceneUpdater::markScenesDependantOnModifiedConsumersAsModified(const DataReferenceLinkManager& dataRefLinkManager, const TransformationLinkManager& transfLinkManager, const TextureLinkManager& texLinkManager)
    {
        auto findFirstOfModifiedScenes = [this](const SceneIdVector& v)
        {
            return std::find_if(v.cbegin(), v.cend(), [this](SceneId a) {return m_modifiedScenesToRerender.contains(a); });
        };

        const auto& transDependencyOrderedScenes = transfLinkManager.getDependencyChecker().getDependentScenesInOrder();
        const auto& dataRefDependencyOrderedScenes = dataRefLinkManager.getDependencyChecker().getDependentScenesInOrder();
        const auto& texDependencyOrderedScenes = texLinkManager.getDependencyChecker().getDependentScenesInOrder();

        const auto transDepRootIt     = findFirstOfModifiedScenes(transDependencyOrderedScenes);
        const auto dataRefDepRootIt   = findFirstOfModifiedScenes(dataRefDependencyOrderedScenes);
        const auto texDepRootIt       = findFirstOfModifiedScenes(texDependencyOrderedScenes);

        m_modifiedScenesToRerender.insert(transDepRootIt,     transDependencyOrderedScenes.cend());
        m_modifiedScenesToRerender.insert(dataRefDepRootIt,   dataRefDependencyOrderedScenes.cend());
        m_modifiedScenesToRerender.insert(texDepRootIt,       texDependencyOrderedScenes.cend());
    }

    void RendererSceneUpdater::markScenesDependantOnModifiedOffscreenBuffersAsModified(const TextureLinkManager& texLinkManager)
    {
        auto findOffscreenBufferSceneIsMappedTo = [this](SceneId sceneId)
        {
            const auto displayBuffer = m_renderer.getBufferSceneIsAssignedTo(sceneId);
            return m_displayResourceManager->getOffscreenBufferHandle(displayBuffer);
        };

        //initially mark all modified scenes as to be visited
        assert(m_offscreeenBufferModifiedScenesVisitingCache.empty());
        m_offscreeenBufferModifiedScenesVisitingCache.reserve(m_modifiedScenesToRerender.size());
        for (const auto& s : m_modifiedScenesToRerender)
            m_offscreeenBufferModifiedScenesVisitingCache.push_back(s);

        //for every scene in the visiting cache: if it render into an OB: mark all scenes that consume the OB as modified
        while (!m_offscreeenBufferModifiedScenesVisitingCache.empty())
        {
            const SceneId sceneId = m_offscreeenBufferModifiedScenesVisitingCache.back();
            m_offscreeenBufferModifiedScenesVisitingCache.pop_back();

            if (m_sceneStateExecutor.getSceneState(sceneId) == ESceneState::Rendered)
            {
                m_modifiedScenesToRerender.put(sceneId);
                // if rendered to offscreen buffer, mark all consumers of that offscreen buffer as modified
                const auto bufferHandle = findOffscreenBufferSceneIsMappedTo(sceneId);
                if (bufferHandle.isValid())
                {
                    m_offscreenBufferConsumerSceneLinksCache.clear();
                    texLinkManager.getOffscreenBufferLinks().getLinkedConsumers(bufferHandle, m_offscreenBufferConsumerSceneLinksCache);

                    for (const auto& link : m_offscreenBufferConsumerSceneLinksCache)
                        if (!m_modifiedScenesToRerender.contains(link.consumerSceneId))
                            m_offscreeenBufferModifiedScenesVisitingCache.push_back(link.consumerSceneId);
                }
            }
        }
    }

    void RendererSceneUpdater::logMissingResources(const PendingData& pendingData, SceneId sceneId) const
    {
        ResourceContentHashVector missingResources;
        for (const auto& pendingFlush : pendingData.pendingFlushes)
            missingResources.insert(missingResources.end(), pendingFlush.resourcesAdded.cbegin(), pendingFlush.resourcesAdded.cend());
        logMissingResources(missingResources, sceneId);
    }

    void RendererSceneUpdater::logMissingResources(const ResourceContentHashVector& neededResources, SceneId sceneId) const
    {
        assert(m_displayResourceManager);
        const IRendererResourceManager& resourceManager = *m_displayResourceManager;

        constexpr UInt MaxMissingResourcesToLog = 10u;

        LOG_ERROR_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos)
        {
            ResourceContentHashVector missingResourcesToLog;
            missingResourcesToLog.reserve(MaxMissingResourcesToLog);
            UInt numMissingResources = 0u;

            for (const auto& res : neededResources)
            {
                if (resourceManager.getResourceStatus(res) != EResourceStatus::Uploaded)
                {
                    if (missingResourcesToLog.size() < MaxMissingResourcesToLog)
                        missingResourcesToLog.push_back(res);
                    numMissingResources++;
                }
            }

            sos << "Missing resources for scene " << sceneId << ": " << numMissingResources << "\n";
            for (const auto& res : missingResourcesToLog)
            {
                sos << " [hash: " << res
                    << "; " << resourceManager.getResourceStatus(res)
                    << "; " << EnumToString(resourceManager.getResourceType(res))
                    << "]\n";
            }
            if (numMissingResources > missingResourcesToLog.size())
                sos << " ...\n";
        }));
    }

    uint32_t RendererSceneUpdater::getNumberOfPendingNonEmptyFlushes(SceneId sceneId) const
    {
        const auto& pendingFlushes = m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes;
        const auto numFlushes = std::count_if(pendingFlushes.cbegin(), pendingFlushes.cend(), [](const auto& pendingFlush) {
            return pendingFlush.sceneActions.numberOfActions() > 0u;
        });

        return static_cast<uint32_t>(numFlushes);
    }

    void RendererSceneUpdater::processScreenshotResults()
    {
        if (!m_displayResourceManager)
            return;

        const IRendererResourceManager& resourceManager = *m_displayResourceManager;
        auto screenshots = m_renderer.dispatchProcessedScreenshots();
        for (auto& bufferScreenshot : screenshots)
        {
            const DeviceResourceHandle renderTargetHandle = bufferScreenshot.first;
            auto& screenshot = bufferScreenshot.second;

            if (!screenshot.filename.empty())
            {
                // flip image vertically so that the layout read from frame buffer (bottom-up)
                // is converted to layout normally used in image files (top-down)
                const Image bitmap(screenshot.rectangle.width, screenshot.rectangle.height, screenshot.pixelData.cbegin(), screenshot.pixelData.cend(), true);
                bitmap.saveToFilePNG(screenshot.filename);
                LOG_INFO(CONTEXT_RENDERER, "RendererSceneUpdater::processScreenshotResults: screenshot successfully saved to file: " << screenshot.filename);
                if (screenshot.sendViaDLT)
                {
                    if (GetRamsesLogger().transmitFile(screenshot.filename, false))
                    {
                        LOG_INFO(CONTEXT_RENDERER, "RendererSceneUpdater::processScreenshotResults: started dlt file transfer: " << screenshot.filename);
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_RENDERER, "RendererSceneUpdater::processScreenshotResults: screenshot file could not send via dlt: " << screenshot.filename);
                    }
                }
            }
            else
            {
                const OffscreenBufferHandle obHandle = resourceManager.getOffscreenBufferHandle(renderTargetHandle);
                m_rendererEventCollector.addReadPixelsEvent(ERendererEventType::ReadPixelsFromFramebuffer, m_display, obHandle, std::move(screenshot.pixelData));
            }
        }
    }
}
