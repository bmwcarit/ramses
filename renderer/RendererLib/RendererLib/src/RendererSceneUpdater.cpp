//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationSystemFactory.h"
#include "Scene/SceneActionApplier.h"
#include "Scene/SceneResourceChanges.h"
#include "Scene/SceneResourceUtils.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/ISurface.h"
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
#include "RendererLib/PendingClientResourcesUtils.h"
#include "RendererLib/PendingSceneResourcesUtils.h"
#include "RendererLib/RendererLogger.h"
#include "RendererLib/IntersectionUtils.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RendererEventCollector.h"
#include "Components/FlushTimeInformation.h"
#include "Components/SceneUpdate.h"
#include "Utils/LogMacros.h"
#include "Utils/Image.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/Macros.h"
#include "absl/algorithm/container.h"

namespace ramses_internal
{
    RendererSceneUpdater::RendererSceneUpdater(
        Renderer& renderer,
        RendererScenes& rendererScenes,
        SceneStateExecutor& sceneStateExecutor,
        RendererEventCollector& eventCollector,
        FrameTimer& frameTimer,
        SceneExpirationMonitor& expirationMonitor,
        IRendererResourceCache* rendererResourceCache)
        : m_renderer(renderer)
        , m_rendererScenes(rendererScenes)
        , m_sceneStateExecutor(sceneStateExecutor)
        , m_rendererEventCollector(eventCollector)
        , m_frameTimer(frameTimer)
        , m_expirationMonitor(expirationMonitor)
        , m_rendererResourceCache(rendererResourceCache)
        , m_animationSystemFactory(EAnimationSystemOwner_Renderer)
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

        while (0u != m_displayResourceManagers.size())
        {
            const DisplayHandle display = m_displayResourceManagers.begin()->first;
            destroyDisplayContext(display);
        }
    }

    void RendererSceneUpdater::handleSceneActions(SceneId sceneId, SceneUpdate&& sceneUpdate)
    {
        ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneId);

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
            LOG_ERROR(CONTEXT_RENDERER, "    RendererSceneUpdater::handleSceneActions could not apply scene actions because scene " << sceneId << " is neither subscribed nor mapped");
    }

    void RendererSceneUpdater::createDisplayContext(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle)
    {
        assert(m_displayResourceManagers.count(handle) == 0);
        m_renderer.resetRenderInterruptState();
        m_renderer.createDisplayContext(displayConfig, handle);

        if (m_renderer.hasDisplayController(handle))
        {
            IDisplayController& displayController = m_renderer.getDisplayController(handle);
            IRenderBackend& renderBackend = displayController.getRenderBackend();
            IEmbeddedCompositingManager& embeddedCompositingManager = displayController.getEmbeddedCompositingManager();

            // ownership of uploadStrategy is transferred into RendererResourceManager
            auto resourceManager = createResourceManager(resourceProvider, resourceUploader, renderBackend, embeddedCompositingManager, handle, displayConfig.getKeepEffectsUploaded(), displayConfig.getGPUMemoryCacheSize());
            m_displayResourceManagers.insert({ handle, std::move(resourceManager) });
            m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayCreated, handle);

            LOG_INFO(CONTEXT_RENDERER, "Created display " << handle.asMemoryHandle() << ": " << displayController.getDisplayWidth() << "x" << displayController.getDisplayHeight()
                << (displayConfig.getFullscreenState() ? " fullscreen" : "") << (displayConfig.isWarpingEnabled() ? " warped" : "") << " MSAA" << displayConfig.getAntialiasingSampleCount());
        }
        else
        {
            m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayCreateFailed, handle);
        }
    }

    std::unique_ptr<IRendererResourceManager> RendererSceneUpdater::createResourceManager(
        IResourceProvider& resourceProvider,
        IResourceUploader& resourceUploader,
        IRenderBackend& renderBackend,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        DisplayHandle display,
        bool keepEffectsUploaded,
        uint64_t gpuCacheSize)
    {
        return std::make_unique<RendererResourceManager>(
            resourceProvider,
            resourceUploader,
            renderBackend,
            embeddedCompositingManager,
            ResourceRequesterID(display.asMemoryHandle()),
            keepEffectsUploaded,
            m_frameTimer,
            m_renderer.getStatistics(),
            gpuCacheSize);
    }

    void RendererSceneUpdater::destroyDisplayContext(DisplayHandle display)
    {
        if (!m_renderer.hasDisplayController(display))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::destroyDisplayContext cannot destroy display " << display << " which does not exist");
            m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayDestroyFailed, display);
            return;
        }
        assert(m_displayResourceManagers.count(display));

        Bool displayHasMappedScene = false;
        for (const auto& it : m_scenesToBeMapped)
        {
            if (it.value.display == display)
            {
                displayHasMappedScene = true;
                break;
            }
        }

        if (!displayHasMappedScene)
        {
            for (const auto& it : m_rendererScenes)
            {
                const SceneId sceneId = it.key;
                if (m_renderer.getDisplaySceneIsAssignedTo(sceneId) == display)
                {
                    displayHasMappedScene = true;
                    break;
                }
            }
        }

        if (displayHasMappedScene)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::destroyDisplayContext cannot destroy display " << display << ", there is one or more scenes mapped (or being mapped) to it, unmap all scenes from it first.");
            m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayDestroyFailed, display);
            return;
        }

        // Context has to be enabled before destruction of resource manager and display controller
        DisplayHandle activeDisplay;
        activateDisplayContext(activeDisplay, display);
        m_displayResourceManagers.erase(display);

        m_renderer.resetRenderInterruptState();
        m_renderer.destroyDisplayContext(display);
        assert(!m_renderer.hasDisplayController(display));
        m_rendererEventCollector.addDisplayEvent(ERendererEventType_DisplayDestroyed, display);
    }

    void RendererSceneUpdater::updateScenes()
    {
        // Display context is activated on demand, assuming that normally at most one scene/display needs resources uploading
        DisplayHandle activeDisplay;

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes request resources from network, upload used resources and unload obsolete resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateClientResources);
            requestAndUploadAndUnloadResources(activeDisplay);
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes try to apply pending flushes, only apply sync flushes if all resources available");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::ApplySceneActions);
            tryToApplyPendingFlushes();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes executing pending scene reference commands and updates states");
            assert(m_sceneReferenceLogic);
            m_sceneReferenceLogic->update();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes unref obsolete client resources and upload pending scene resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateSceneResources);
            processStagedResourceChangesFromAppliedFlushes(activeDisplay);
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update embedded compositing resources");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateEmbeddedCompositingResources);
            updateEmbeddedCompositingResources(activeDisplay);
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes stream texture dirtiness");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateStreamTextures);
            updateSceneStreamTexturesDirtiness();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes to be mapped/shown");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateScenesToBeMapped);
            updateScenesStates();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes resource cache");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateResourceCache);
            updateScenesResourceCache();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes real-time animation systems");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateAnimations);
            updateScenesRealTimeAnimationSystems();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes transformation cache and transformation links");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateTransformations);
            updateScenesTransformationCache();
        }

        {
            LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::updateScenes update scenes data links");
            FRAME_PROFILER_REGION(FrameProfilerStatistics::ERegion::UpdateDataLinks);
            updateScenesDataLinks();
        }

        for (const auto scene : m_modifiedScenesToRerender)
        {
            if (m_sceneStateExecutor.getSceneState(scene) == ESceneState::Rendered)
                m_renderer.markBufferWithSceneAsModified(scene);
        }
        m_modifiedScenesToRerender.clear();
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
        assert(numActions > 0);
        Bool hasSizeInfo = false;
        SceneResourceChanges resourceChanges;
        SceneActionApplier::ReadParameterForFlushAction(sceneUpdate.actions.back(), flushInfo.flushIndex, hasSizeInfo, stagingInfo.sizeInformation, resourceChanges, pendingData.sceneReferenceActions, flushInfo.timeInfo, flushInfo.versionTag);

        if (sceneUpdate.flushInfos.containsValidInformation &&
            !(flushInfo.flushIndex == sceneUpdate.flushInfos.flushCounter &&
                hasSizeInfo == sceneUpdate.flushInfos.hasSizeInfo &&
                flushInfo.timeInfo.clock_type == sceneUpdate.flushInfos.flushTimeInfo.clock_type &&
                std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfo.timeInfo.expirationTimestamp) == std::chrono::time_point_cast<std::chrono::milliseconds>(sceneUpdate.flushInfos.flushTimeInfo.expirationTimestamp) &&
                std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfo.timeInfo.internalTimestamp) == std::chrono::time_point_cast<std::chrono::milliseconds>(sceneUpdate.flushInfos.flushTimeInfo.internalTimestamp) &&
                flushInfo.versionTag == sceneUpdate.flushInfos.versionTag))
        {
            LOG_WARN_P(CONTEXT_RENDERER, "FlushInformation of SceneUpdate differs from legacy scene action flush data. {} != legacy:flushcounter{};version:{};time[{};exp:{};int:{}];sizeInfo:{}",
                sceneUpdate.flushInfos,
                flushInfo.flushIndex, flushInfo.versionTag, flushInfo.timeInfo.clock_type,
                static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfo.timeInfo.expirationTimestamp).time_since_epoch().count()),
                static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfo.timeInfo.internalTimestamp).time_since_epoch().count()),
                hasSizeInfo);
            assert(false);
        }

        // get ptp synchronized time and check current and receive times for validity
        const auto flushConsolidateTs = FlushTime::Clock::now();
        if (flushConsolidateTs != FlushTime::InvalidTimestamp &&
            flushInfo.timeInfo.internalTimestamp != FlushTime::InvalidTimestamp)
        {
            // collect latency timing statistics between flush call on Scene and here
            const auto flushLatencyMs = std::chrono::duration_cast<std::chrono::milliseconds>(flushConsolidateTs - flushInfo.timeInfo.internalTimestamp);
            m_renderer.getStatistics().trackArrivedFlush(sceneID, numActions, resourceChanges.m_addedClientResourceRefs.size(), resourceChanges.m_removedClientResourceRefs.size(), resourceChanges.m_sceneResourceActions.size(), flushLatencyMs);
        }

        LOG_TRACE_F(CONTEXT_RENDERER, ([&](StringOutputStream& logStream) {
            logStream << "Flush " << flushInfo.flushIndex << " for scene " << sceneID << " arrived ";
            logStream << "[actions:" << numActions << "(" << sceneUpdate.actions.collectionData().size() << " bytes)]";
            logStream << "[addRefs res (" << resourceChanges.m_addedClientResourceRefs.size() << "):";
            for (const auto& hash : resourceChanges.m_addedClientResourceRefs)
                logStream << " " << hash;
            logStream << "]";
            logStream << "[removeRefs res (" << resourceChanges.m_removedClientResourceRefs.size() << "):";
            for (const auto& hash : resourceChanges.m_removedClientResourceRefs)
                logStream << " " << hash;
            logStream << "]";
            logStream << "[scene res actions:" << resourceChanges.m_sceneResourceActions.size() << "]";
            if (hasSizeInfo)
            {
                logStream << " " << stagingInfo.sizeInformation.asString().c_str();
            }
        }));
        if (!resourceChanges.empty())
        {
            LOG_TRACE(CONTEXT_RENDERER, resourceChanges.asString());
        }

        // TODO vaclav store and use pushed resources
        assert(sceneUpdate.resources.empty());
        //for (const auto& addedRes : resourceChanges.m_addedClientResourceRefs)
        //{
        //    assert(absl::c_find_if(sceneUpdate.resources, [&addedRes](const ManagedResource& mr) {return mr.getResourceObject()->getHash() == addedRes; }) != sceneUpdate.resources.cend());
        //    UNUSED(addedRes);
        //}

        ResourceContentHashVector newlyNeededClientResources;
        consolidateResourceChanges(stagingInfo.pendingData, resourceChanges, newlyNeededClientResources);

        // add references to newly needed client resources right away
        if (!newlyNeededClientResources.empty())
        {
            const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneID);
            if (displayHandle.isValid())
            {
                IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;
                resourceManager.referenceClientResourcesForScene(sceneID, newlyNeededClientResources);
            }
        }

        flushInfo.sceneActions = std::move(sceneUpdate.actions);

        if (pendingFlushes.size() > m_maximumPendingFlushesToKillScene)
            logTooManyFlushesAndUnsubscribeIfRemoteScene(sceneID, stagingInfo.pendingData.pendingFlushes.size());
    }

    void RendererSceneUpdater::consolidateResourceChanges(PendingData& pendingData, const SceneResourceChanges& resourceChanges, ResourceContentHashVector& newlyNeededClientResources) const
    {
        if (!resourceChanges.m_addedClientResourceRefs.empty() || !resourceChanges.m_removedClientResourceRefs.empty())
        {
            ResourceContentHashVector newlyUnneededPreviouslyNeededResources;
            PendingClientResourcesUtils::ConsolidateAddedResources(pendingData.clientResourcesNeeded, pendingData.clientResourcesUnneeded, newlyNeededClientResources, resourceChanges.m_addedClientResourceRefs);
            PendingClientResourcesUtils::ConsolidateRemovedResources(pendingData.clientResourcesNeeded, pendingData.clientResourcesUnneeded, newlyUnneededPreviouslyNeededResources, resourceChanges.m_removedClientResourceRefs);
            PendingClientResourcesUtils::ConsolidateNewlyNeededResources(newlyNeededClientResources, pendingData.clientResourcesPendingUnneeded);
            PendingClientResourcesUtils::ConsolidateNewlyUnneededResources(pendingData.clientResourcesPendingUnneeded, newlyUnneededPreviouslyNeededResources);
        }

        PendingSceneResourcesUtils::ConsolidateSceneResourceActions(resourceChanges.m_sceneResourceActions, pendingData.sceneResourceActions);
    }

    void RendererSceneUpdater::requestAndUploadAndUnloadResources(DisplayHandle& activeDisplay)
    {
        // request newly referenced resources
        // and upload and unload pending resources
        for(const auto& it : m_displayResourceManagers)
        {
            const DisplayHandle displayHandle = it.first;
            IRendererResourceManager& resourceManager = *it.second;

            resourceManager.getRequestedResourcesAlreadyInCache(m_rendererResourceCache);
            resourceManager.requestAndUnrequestPendingClientResources();
            resourceManager.processArrivedClientResources(m_rendererResourceCache);

            if (resourceManager.hasClientResourcesToBeUploaded())
            {
                activateDisplayContext(activeDisplay, displayHandle);
                resourceManager.uploadAndUnloadPendingClientResources();
            }
        }
    }

    void RendererSceneUpdater::updateEmbeddedCompositingResources(DisplayHandle& activeDisplay)
    {
        for(const auto& displayResourceManager : m_displayResourceManagers)
        {
            const DisplayHandle displayHandle = displayResourceManager.first;
            assert(m_renderer.hasDisplayController(displayHandle));

            IEmbeddedCompositingManager& embeddedCompositingManager = m_renderer.getDisplayController(displayHandle).getEmbeddedCompositingManager();
            //TODO Mohamed: remove this if statement as soon as EC dummy is removed
            if(embeddedCompositingManager.hasRealCompositor())
            {
                embeddedCompositingManager.processClientRequests();
                if (embeddedCompositingManager.hasUpdatedContentFromStreamSourcesToUpload())
                {
                    activateDisplayContext(activeDisplay, displayHandle);
                    StreamTextureBufferUpdates bufferUpdates;
                    embeddedCompositingManager.uploadResourcesAndGetUpdates(m_modifiedScenesToRerender, bufferUpdates);

                    for (const auto& streamTextureBufferUpdate : bufferUpdates)
                    {
                        m_renderer.getStatistics().streamTextureUpdated(streamTextureBufferUpdate.key, streamTextureBufferUpdate.value);
                    }
                }
            }
        }
    }

    void RendererSceneUpdater::tryToApplyPendingFlushes()
    {
        UInt32 numActionsAppliedForStatistics = 0;

        // check and try to apply pending flushes
        for(const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneID = rendererScene.key;
            StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneID);

            if (!stagingInfo.pendingData.pendingFlushes.empty())
            {
                numActionsAppliedForStatistics += updateScenePendingFlushes(sceneID, stagingInfo);
            }
        }

        m_renderer.getProfilerStatistics().setCounterValue(FrameProfilerStatistics::ECounter::AppliedSceneActions, numActionsAppliedForStatistics);
    }

    UInt32 RendererSceneUpdater::updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo)
    {
        const ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneID);
        const Bool sceneIsRenderedOrRequested = (sceneState == ESceneState::Rendered || sceneState == ESceneState::RenderRequested); // requested can become rendered still in this frame
        const Bool sceneIsMapped = (sceneState == ESceneState::Mapped) || sceneIsRenderedOrRequested;
        const Bool sceneIsMappedOrMapping = (sceneState == ESceneState::MappingAndUploading) || sceneIsMapped;
        const Bool resourcesReady = sceneIsMappedOrMapping && willApplyingChangesMakeAllResourcesAvailable(sceneID);

        Bool canApplyFlushes = !sceneIsMappedOrMapping || resourcesReady;

        if (sceneIsRenderedOrRequested && m_renderer.hasAnyBufferWithInterruptedRendering())
            canApplyFlushes &= !m_renderer.isSceneAssignedToInterruptibleOffscreenBuffer(sceneID);

        const PendingFlushes& pendingFlushes = stagingInfo.pendingData.pendingFlushes;
        if (!canApplyFlushes && sceneIsMapped && pendingFlushes.size() > m_maximumPendingFlushes)
        {
            LOG_ERROR(CONTEXT_RENDERER, "Force applying pending flushes! Scene " << sceneID << " has " << pendingFlushes.size() << " pending flushes, renderer cannot catch up with resource updates.");
            logMissingResources(stagingInfo.pendingData.clientResourcesNeeded, sceneID);

            canApplyFlushes = true;
            m_renderer.resetRenderInterruptState();
        }

        if (canApplyFlushes)
        {
            const EResourceStatus resourcesStatus = (resourcesReady ? EResourceStatus_Uploaded : EResourceStatus_Unknown);
            stagingInfo.pendingData.allPendingFlushesApplied = true;
            return applyPendingFlushes(sceneID, stagingInfo, resourcesStatus);
        }
        else
            m_renderer.getStatistics().flushBlocked(sceneID);

        return 0;
    }

    UInt32 RendererSceneUpdater::applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo, EResourceStatus resourcesStatus)
    {
        IScene& rendererScene = const_cast<RendererCachedScene&>(m_rendererScenes.getScene(sceneID));
        rendererScene.preallocateSceneSize(stagingInfo.sizeInformation);

        PendingData& pendingData = stagingInfo.pendingData;
        PendingFlushes& pendingFlushes = pendingData.pendingFlushes;
        UInt numActionsApplied = 0u;
        for (auto& pendingFlush : pendingFlushes)
        {
            applySceneActions(rendererScene, pendingFlush);

            numActionsApplied += pendingFlush.sceneActions.numberOfActions();

            if (pendingFlush.versionTag.isValid())
            {
                LOG_INFO(CONTEXT_SMOKETEST, "Named flush applied on scene " << rendererScene.getSceneId() <<
                    " with sceneVersionTag " << pendingFlush.versionTag);
                m_rendererEventCollector.addSceneFlushEvent(ERendererEventType_SceneFlushed, sceneID, pendingFlush.versionTag, resourcesStatus);
            }
            stagingInfo.lastAppliedVersionTag = pendingFlush.versionTag;
            m_expirationMonitor.onFlushApplied(sceneID, pendingFlush.timeInfo.expirationTimestamp, pendingFlush.versionTag, pendingFlush.flushIndex);
            m_renderer.getStatistics().flushApplied(sceneID);

            // mark scene as modified only if it received scene actions other than those below
            static const std::vector<ESceneActionId> SceneActionsIgnoredForMarkingAsModified = { ESceneActionId::Flush, ESceneActionId::SetAckFlushState };
            const bool isFlushWithChanges = std::any_of(pendingFlush.sceneActions.begin(), pendingFlush.sceneActions.end(),
                [](const SceneActionCollection::SceneActionReader& a) { return !contains_c(SceneActionsIgnoredForMarkingAsModified, a.type()); });
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

        return static_cast<UInt32>(numActionsApplied);
    }

    void RendererSceneUpdater::processStagedResourceChangesFromAppliedFlushes(DisplayHandle& activeDisplay)
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
                processStagedResourceChanges(sceneID, stagingInfo, activeDisplay);
                PendingData::Clear(pendingData);
            }
        }
    }

    void RendererSceneUpdater::processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo, DisplayHandle& activeDisplay)
    {
        // if scene is mapped unreference client resources that are no longer needed
        // and execute collected scene resource actions
        const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneID);
        PendingData& pendingData = stagingInfo.pendingData;
        if (displayHandle.isValid())
        {
            IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;
            resourceManager.unreferenceClientResourcesForScene(sceneID, pendingData.clientResourcesUnneeded);
            resourceManager.unreferenceClientResourcesForScene(sceneID, pendingData.clientResourcesPendingUnneeded);

            const auto& pendingSceneResourceActions = pendingData.sceneResourceActions;
            if (!pendingSceneResourceActions.empty())
            {
                activateDisplayContext(activeDisplay, displayHandle);
                PendingSceneResourcesUtils::ApplySceneResourceActions(pendingSceneResourceActions, m_rendererScenes.getScene(sceneID), resourceManager);
            }
        }

        // Pending flush(es) were applied, update the list of resources in use (regardless of scene being mapped or not),
        // consolidate needed/unneeded resources for the applied flush(es)
        PendingClientResourcesUtils::ConsolidateNeededAndUnneededResources(stagingInfo.clientResourcesInUse, pendingData.clientResourcesNeeded, pendingData.clientResourcesUnneeded);
    }

    void RendererSceneUpdater::updateSceneStreamTexturesDirtiness()
    {
        for(const auto& displayResourceManager : m_displayResourceManagers)
        {
            const DisplayHandle displayHandle = displayResourceManager.first;
            assert(m_renderer.hasDisplayController(displayHandle));

            IEmbeddedCompositingManager& embeddedCompositingManager = m_renderer.getDisplayController(displayHandle).getEmbeddedCompositingManager();
            //TODO Mohamed: remove this if statement as soon as EC dummy is removed
            if(embeddedCompositingManager.hasRealCompositor())
            {
                SceneStreamTextures updatedStreamTextures;
                StreamTextureSourceIdVector newStreams;
                StreamTextureSourceIdVector obsoleteStreams;
                embeddedCompositingManager.dispatchStateChangesOfStreamTexturesAndSources(updatedStreamTextures, newStreams, obsoleteStreams);

                for (const auto stream : newStreams)
                {
                    m_rendererEventCollector.addStreamSourceEvent(ERendererEventType_StreamSurfaceAvailable, stream);
                }
                for (const auto stream : obsoleteStreams)
                {
                    m_rendererEventCollector.addStreamSourceEvent(ERendererEventType_StreamSurfaceUnavailable, stream);
                    m_renderer.getStatistics().untrackStreamTexture(stream);
                }

                for(const auto& updatedStreamTexture : updatedStreamTextures)
                {
                    const SceneId sceneId = updatedStreamTexture.key;
                    const RendererCachedScene& rendererScene = m_rendererScenes.getScene(sceneId);

                    const StreamTextureHandleVector& streamTexturesPerScene = updatedStreamTexture.value;
                    for(const auto& streamTexture : streamTexturesPerScene)
                    {
                        rendererScene.setRenderableResourcesDirtyByStreamTexture(streamTexture);
                    }

                    m_modifiedScenesToRerender.put(sceneId);
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
                const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
                assert(displayHandle.isValid());
                const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;
                const IEmbeddedCompositingManager& embeddedCompositingManager = m_renderer.getDisplayController(displayHandle).getEmbeddedCompositingManager();
                RendererCachedScene& rendererScene = *(sceneIt.value.scene);
                rendererScene.updateRenderablesAndResourceCache(resourceManager, embeddedCompositingManager);
            }
        }
    }

    void RendererSceneUpdater::updateScenesStates()
    {
        SceneIdVector scenesMapped;
        SceneIdVector scenesToForceUnsubscribe;
        for (const auto& it : m_scenesToBeMapped)
        {
            const SceneId sceneId = it.key;
            const SceneMapRequest& mapRequest = it.value;
            const ESceneState sceneState = m_sceneStateExecutor.getSceneState(sceneId);

            switch (sceneState)
            {
            case ESceneState::MapRequested:
            {
                assert(m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes.empty());
                const IDisplayController& displayController = m_renderer.getDisplayController(mapRequest.display);
                m_renderer.assignSceneToDisplayBuffer(sceneId, mapRequest.display, displayController.getDisplayBuffer(), 0);
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
                assert(m_renderer.getDisplaySceneIsAssignedTo(sceneId) == mapRequest.display);

                const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(mapRequest.display)->second;
                const StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneId);

                bool canBeMapped = false;
                // allow map only if there are no pending flushes
                if (stagingInfo.pendingData.pendingFlushes.empty())
                {
                    canBeMapped = true;
                    for (const auto& res : stagingInfo.clientResourcesInUse)
                    {
                        if (resourceManager.getClientResourceStatus(res) != EResourceStatus_Uploaded)
                        {
                            canBeMapped = false;
                            break;
                        }
                    }
                }

                if (!canBeMapped && stagingInfo.pendingData.pendingFlushes.size() > m_maximumPendingFlushes)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "Force mapping scene " << sceneId << " due to " << stagingInfo.pendingData.pendingFlushes.size() << " pending flushes, renderer cannot catch up with resource updates.");
                    logMissingResources(stagingInfo.clientResourcesInUse, sceneId);

                    canBeMapped = true;
                }

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
            m_scenesToBeMapped.remove(sceneId);

            const auto& pendingFlushes = m_rendererScenes.getStagingInfo(sceneId).pendingData.pendingFlushes;
            if (!pendingFlushes.empty())
            {
                LOG_ERROR(CONTEXT_RENDERER, "Scene " << sceneId << " - expected no pending flushes at this point");
                assert(pendingFlushes.size() > m_maximumPendingFlushes);
            }
        }

        // check scenes that take too long to be mapped
        for (auto& sceneMapReq : m_scenesToBeMapped)
        {
            constexpr std::chrono::seconds MappingLogPeriod{ 1u };
            constexpr size_t MaxNumResourcesToLog = 20u;

            auto& mapRequest = sceneMapReq.value;
            const auto currentFrameTime = m_frameTimer.getFrameStartTime();
            if (currentFrameTime - mapRequest.lastLogTimeStamp > MappingLogPeriod)
            {
                LOG_WARN_F(CONTEXT_RENDERER, [&](ramses_internal::StringOutputStream& logger)
                {
                    const auto totalWaitingTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTime - mapRequest.requestTimeStamp);
                    const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(mapRequest.display)->second;
                    const StagingInfo& stagingInfo = m_rendererScenes.getStagingInfo(sceneMapReq.key);

                    logger << "Scene " << sceneMapReq.key << " waiting " << totalWaitingTime.count() << " ms for resources in order to be mapped: ";
                    size_t numResourcesWaiting = 0u;
                    for (const auto& res : stagingInfo.clientResourcesInUse)
                    {
                        const auto resStatus = resourceManager.getClientResourceStatus(res);
                        if (resStatus != EResourceStatus_Uploaded)
                        {
                            if (++numResourcesWaiting <= MaxNumResourcesToLog)
                                logger << res << " <" << EnumToString(resStatus) << ">; ";
                        }
                    }
                    logger << " " << numResourcesWaiting << " unresolved resources in total";
                });

                mapRequest.lastLogTimeStamp = currentFrameTime;

                // log at most 1 scene in one frame
                break;
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

    void RendererSceneUpdater::applySceneActions(IScene& scene, PendingFlush& flushInfo)
    {
        const SceneActionCollection& actionsForScene = flushInfo.sceneActions;
        const UInt32 numActions = actionsForScene.numberOfActions();
        LOG_TRACE(CONTEXT_PROFILING, "    RendererSceneUpdater::applySceneActions start applying scene actions [count:" << numActions << "] for scene with id " << scene.getSceneId());

        SceneActionApplier::ApplyActionsOnScene(scene, actionsForScene, &m_animationSystemFactory);

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

        if (m_scenesToBeMapped.contains(sceneID))
        {
            assert(sceneState == ESceneState::MapRequested || sceneState == ESceneState::MappingAndUploading);
            m_scenesToBeMapped.remove(sceneID);
        }

        m_expirationMonitor.onDestroyed(sceneID);
    }

    void RendererSceneUpdater::unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        assert(SceneStateIsAtLeast(m_sceneStateExecutor.getSceneState(sceneId), ESceneState::MappingAndUploading));
        const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
        assert(displayHandle.isValid());
        IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;

        DisplayHandle activeDisplay;
        activateDisplayContext(activeDisplay, displayHandle);
        resourceManager.unloadAllSceneResourcesForScene(sceneId);
        resourceManager.unreferenceAllClientResourcesForScene(sceneId);

        RendererCachedScene& rendererScene = m_rendererScenes.getScene(sceneId);
        rendererScene.resetResourceCache();
    }

    bool RendererSceneUpdater::markClientAndSceneResourcesForReupload(SceneId sceneId)
    {
        assert(m_rendererScenes.hasScene(sceneId));
        assert(ESceneState::MappingAndUploading == m_sceneStateExecutor.getSceneState(sceneId));

        const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
        assert(displayHandle.isValid());
        IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;

        // collect all scene resources in scene and upload them
        const IScene& scene = m_rendererScenes.getScene(sceneId);
        SceneResourceActionVector sceneResourceActions;
        size_t sceneResourcesByteSize = 0u;
        SceneResourceUtils::GetAllSceneResourcesFromScene(sceneResourceActions, scene, sceneResourcesByteSize);
        if (!sceneResourceActions.empty())
        {
            DisplayHandle activeDisplay;
            activateDisplayContext(activeDisplay, displayHandle);
            if (sceneResourcesByteSize > 0u)
                LOG_INFO(CONTEXT_RENDERER, "Applying scene resources gathered from scene " << sceneId << ", " << sceneResourceActions.size() << " actions, " << sceneResourcesByteSize << " bytes");

            // enable time measuring and interrupting of upload only if scene is remote
            const bool sceneIsRemote = (m_sceneStateExecutor.getScenePublicationMode(sceneId) != EScenePublicationMode_LocalOnly);
            if (!PendingSceneResourcesUtils::ApplySceneResourceActions(sceneResourceActions, scene, resourceManager, sceneIsRemote ? &m_frameTimer : nullptr))
                return false;
        }

        // reference all the resources in use by the scene to be mapped
        const auto& clientResources = m_rendererScenes.getStagingInfo(sceneId).clientResourcesInUse;
        if (!clientResources.empty())
            LOG_INFO(CONTEXT_RENDERER, "Marking " << clientResources.size() << " client resources as used by scene " << sceneId << ", resources which are not yet available will be requested/loaded and uploaded to GPU.");
        resourceManager.referenceClientResourcesForScene(sceneId, m_rendererScenes.getStagingInfo(sceneId).clientResourcesInUse);
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

    void RendererSceneUpdater::handleSceneMappingRequest(SceneId sceneId, DisplayHandle handle)
    {
        if (m_sceneStateExecutor.checkIfCanBeMapRequested(sceneId, handle))
        {
            m_sceneStateExecutor.setMapRequested(sceneId, handle);
            assert(!m_scenesToBeMapped.contains(sceneId));
            m_scenesToBeMapped.put(sceneId, { handle, m_frameTimer.getFrameStartTime(), m_frameTimer.getFrameStartTime() });
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
                m_rendererEventCollector.addInternalSceneEvent(ERendererEventType_SceneMapFailed, sceneId);
                m_scenesToBeMapped.remove(sceneId);
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
            m_rendererEventCollector.addInternalSceneEvent(ERendererEventType_SceneShowFailed, sceneId);
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

    Bool RendererSceneUpdater::handleBufferCreateRequest(OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, Bool isDoubleBuffered)
    {
        if (!m_displayResourceManagers.count(display))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferCreateRequest cannot create an offscreen buffer on unknown display " << display);
            return false;
        }

        IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(display)->second;
        if (resourceManager.getOffscreenBufferDeviceHandle(buffer).isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferCreateRequest an offscreen buffer with the same ID (" << buffer << ") already exists!");
            return false;
        }

        DisplayHandle activeDisplay;
        activateDisplayContext(activeDisplay, display);
        resourceManager.uploadOffscreenBuffer(buffer, width, height, isDoubleBuffered);
        const DeviceResourceHandle deviceHandle = resourceManager.getOffscreenBufferDeviceHandle(buffer);
        m_renderer.resetRenderInterruptState();
        m_renderer.registerOffscreenBuffer(display, deviceHandle, width, height, isDoubleBuffered);

        LOG_INFO(CONTEXT_RENDERER, "Created offscreen buffer " << buffer.asMemoryHandle() << " (device handle " << deviceHandle.asMemoryHandle() << "): " << width << "x" << height
            << (isDoubleBuffered ? " interruptible" : ""));

        return true;
    }

    Bool RendererSceneUpdater::handleBufferDestroyRequest(OffscreenBufferHandle buffer, DisplayHandle display)
    {
        if (!m_displayResourceManagers.count(display))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest cannot destroy an offscreen buffer on unknown display " << display);
            return false;
        }

        IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(display)->second;
        const auto bufferDeviceHandle = resourceManager.getOffscreenBufferDeviceHandle(buffer);
        if (!bufferDeviceHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest could not find buffer with ID " << buffer << " on given display " << display);
            return false;
        }

        for (const auto& rendererScene : m_rendererScenes)
        {
            const SceneId sceneId = rendererScene.key;
            DisplayHandle sceneDisplay;
            const auto sceneDisplayBuffer = m_renderer.getBufferSceneIsAssignedTo(sceneId, &sceneDisplay);
            if (sceneDisplay.isValid())
            {
                assert(SceneStateIsAtLeast(m_sceneStateExecutor.getSceneState(sceneId), ESceneState::MappingAndUploading));

                if (sceneDisplay == display && sceneDisplayBuffer == bufferDeviceHandle)
                {
                    LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleBufferDestroyRequest cannot destroy buffer " << buffer << ", there is one or more scenes assigned to it, unmap or reassign them first.");
                    return false;
                }
            }
        }

        m_rendererScenes.getSceneLinksManager().handleBufferDestroyed(buffer);
        m_renderer.resetRenderInterruptState();
        m_renderer.unregisterOffscreenBuffer(display, bufferDeviceHandle);

        DisplayHandle activeDisplay;
        activateDisplayContext(activeDisplay, display);
        resourceManager.unloadOffscreenBuffer(buffer);

        return true;
    }

    void RendererSceneUpdater::handleSetClearColor(DisplayHandle display, OffscreenBufferHandle buffer, const Vector4& clearColor)
    {
        if (!m_renderer.hasDisplayController(display))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearColor failed, unknown display " << display);
            return;
        }

        DeviceResourceHandle bufferDeviceHandle;
        if (buffer.isValid())
        {
            bufferDeviceHandle = m_displayResourceManagers.find(display)->second->getOffscreenBufferDeviceHandle(buffer);
            if (!bufferDeviceHandle.isValid())
            {
                LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSetClearColor cannot set clear color for unknown offscreen buffer " << buffer);
                return;
            }
        }
        else
            bufferDeviceHandle = m_renderer.getDisplayController(display).getDisplayBuffer();

        assert(bufferDeviceHandle.isValid());
        m_renderer.setClearColor(display, bufferDeviceHandle, clearColor);
    }

    void RendererSceneUpdater::handleReadPixels(DisplayHandle display, OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo)
    {
        bool readPixelsFailed = false;
        DeviceResourceHandle renderTargetHandle{};
        if (m_renderer.hasDisplayController(display))
        {
            const auto& displayResourceManager = *m_displayResourceManagers.find(display)->second;
            const auto& displayController = m_renderer.getDisplayController(display);

            if (buffer.isValid())
                renderTargetHandle = displayResourceManager.getOffscreenBufferDeviceHandle(buffer);
            else
                renderTargetHandle = displayController.getDisplayBuffer();

            if (renderTargetHandle.isValid())
            {
                const auto& bufferViewport = m_renderer.getDisplaySetup(display).getDisplayBuffer(renderTargetHandle).viewport;
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
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::readPixels failed, unknown display " << display.asMemoryHandle());
            readPixelsFailed = true;
        }

        if (readPixelsFailed)
        {
            if (screenshotInfo.filename.empty())
                // only generate event when not saving pixels to file!
                m_rendererEventCollector.addReadPixelsEvent(ERendererEventType_ReadPixelsFromFramebufferFailed, display, buffer, {});
        }
        else
            m_renderer.scheduleScreenshot(display, renderTargetHandle, std::move(screenshotInfo));
    }

    Bool RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, Int32 sceneRenderOrder)
    {
        const DisplayHandle display = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
        if (!display.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest cannot assign scene " << sceneId << " to an offscreen buffer; It must be mapped to a display first!");
            return false;
        }

        const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(display)->second;
        // determine if assigning to display's framebuffer or an offscreen buffer
        const DeviceResourceHandle bufferDeviceHandle = (buffer.isValid() ? resourceManager.getOffscreenBufferDeviceHandle(buffer) : m_renderer.getDisplayController(display).getDisplayBuffer());
        if (!bufferDeviceHandle.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handleSceneDisplayBufferAssignmentRequest could not find buffer " << buffer << " on display " << display << " where scene " << sceneId << " is currently mapped");
            return false;
        }

        m_renderer.resetRenderInterruptState();
        m_renderer.assignSceneToDisplayBuffer(sceneId, display, bufferDeviceHandle, sceneRenderOrder);

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
                const EDataSlotType consumerSlotType = DataLinkUtils::GetDataSlot(consumerSceneId, consumerSlotHandle, m_rendererScenes).type;

                if (providerSlotType == EDataSlotType_TextureProvider && consumerSlotType == EDataSlotType_TextureConsumer)
                {
                    const DisplayHandle providerDisplay = m_renderer.getDisplaySceneIsAssignedTo(providerSceneId);
                    const DisplayHandle consumerDisplay = m_renderer.getDisplaySceneIsAssignedTo(consumerSceneId);
                    if (!providerDisplay.isValid() || !consumerDisplay.isValid()
                        || providerDisplay != consumerDisplay)
                    {
                        LOG_ERROR(CONTEXT_RENDERER, "Renderer::createDataLink failed: both provider and consumer scenes have to be mapped to same display when using texture linking!  (Provider scene: " << providerSceneId << ") (Consumer scene: " << consumerSceneId << ")");
                        m_rendererEventCollector.addDataLinkEvent(ERendererEventType_SceneDataLinkFailed, providerSceneId, consumerSceneId, providerId, consumerId);
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
        const DisplayHandle display = m_renderer.getDisplaySceneIsAssignedTo(consumerSceneId);
        if (!display.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createBufferLink failed: consumer scene (Scene: " << consumerSceneId << ") has to be mapped!");
            m_rendererEventCollector.addOBLinkEvent(ERendererEventType_SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
            return;
        }

        const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(display)->second;
        if (!resourceManager.getOffscreenBufferDeviceHandle(buffer).isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Renderer::createBufferLink failed: offscreen buffer " << buffer << " has to exist on the same display where the consumer scene " << consumerSceneId << " is mapped!");
            m_rendererEventCollector.addOBLinkEvent(ERendererEventType_SceneDataBufferLinkFailed, buffer, consumerSceneId, consumerId);
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
        if (!m_rendererScenes.hasScene(sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handlePickEvent could not process pick event for scene " << sceneId << " which is not known to renderer.");
            return;
        }

        auto display = DisplayHandle::Invalid();
        const auto bufferHandle = m_renderer.getBufferSceneIsAssignedTo(sceneId, &display);
        if (!display.isValid())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererSceneUpdater::handlePickEvent could not process pick event for scene " << sceneId << " because it is not mapped to any display.");
            return;
        }
        assert(bufferHandle.isValid());

        const auto& buffer = m_renderer.getDisplaySetup(display).getDisplayBuffer(bufferHandle);
        const Vector2i coordsInBufferSpace = { static_cast<Int32>(std::lroundf((coordsNormalizedToBufferSize.x + 1.f) * buffer.viewport.width / 2.f)) ,
                                                static_cast<Int32>(std::lroundf((coordsNormalizedToBufferSize.y + 1.f) * buffer.viewport.height / 2.f)) };

        PickableObjectIds pickedObjects;
        const TransformationLinkCachedScene& scene = m_rendererScenes.getScene(sceneId);


        IntersectionUtils::CheckSceneForIntersectedPickableObjects(scene, coordsInBufferSpace, pickedObjects);
        if (!pickedObjects.empty())
            m_rendererEventCollector.addPickedEvent(ERendererEventType_ObjectsPicked, sceneId, std::move(pickedObjects));
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

    void RendererSceneUpdater::setSceneReferenceLogicHandler(ISceneReferenceLogic& sceneRefLogic)
    {
        assert(m_sceneReferenceLogic == nullptr);
        m_sceneReferenceLogic = &sceneRefLogic;
    }

    Bool RendererSceneUpdater::willApplyingChangesMakeAllResourcesAvailable(SceneId sceneId) const
    {
        const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
        const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;

        const auto& neededResources = m_rendererScenes.getStagingInfo(sceneId).pendingData.clientResourcesNeeded;
        for (const auto& res : neededResources)
        {
            if (EResourceStatus_Uploaded != resourceManager.getClientResourceStatus(res))
            {
                return false;
            }
        }

        return true;
    }

    void RendererSceneUpdater::updateScenesRealTimeAnimationSystems()
    {
        const UInt64 systemTime = PlatformTime::GetMillisecondsAbsolute();

        for (const auto& scene : m_rendererScenes)
        {
            const SceneId sceneID = scene.key;
            if (m_sceneStateExecutor.getSceneState(sceneID) == ESceneState::Rendered)
            {
                RendererCachedScene& renderScene = m_rendererScenes.getScene(sceneID);
                for (auto handle = AnimationSystemHandle(0); handle < renderScene.getAnimationSystemCount(); ++handle)
                {
                    if (renderScene.isAnimationSystemAllocated(handle))
                    {
                        IAnimationSystem* animationSystem = renderScene.getAnimationSystem(handle);
                        if (animationSystem->isRealTime())
                        {
                            animationSystem->setTime(systemTime);

                            if (animationSystem->hasActiveAnimations())
                                m_modifiedScenesToRerender.put(sceneID);
                        }
                    }
                }
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
            DisplayHandle displayHandle;
            const auto displayBuffer = m_renderer.getBufferSceneIsAssignedTo(sceneId, &displayHandle);
            const IResourceDeviceHandleAccessor& resMgr = *m_displayResourceManagers[displayHandle];
            return resMgr.getOffscreenBufferHandle(displayBuffer);
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

    void RendererSceneUpdater::logMissingResources(const ResourceContentHashVector& resourceVector, SceneId sceneId) const
    {
        const DisplayHandle displayHandle = m_renderer.getDisplaySceneIsAssignedTo(sceneId);
        assert(displayHandle.isValid());
        const IRendererResourceManager& resourceManager = *m_displayResourceManagers.find(displayHandle)->second;

        constexpr UInt MaxMissingResourcesToLog = 10u;

        LOG_ERROR_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos)
        {
            ResourceContentHashVector missingResourcesToLog;
            missingResourcesToLog.reserve(MaxMissingResourcesToLog);
            UInt numMissingResources = 0u;

            for (const auto& res : resourceVector)
            {
                if (resourceManager.getClientResourceStatus(res) != EResourceStatus_Uploaded)
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
                    << "; " << EnumToString(resourceManager.getClientResourceStatus(res))
                    << "; " << EnumToString(resourceManager.getClientResourceType(res))
                    << "]\n";
            }
            if (numMissingResources > missingResourcesToLog.size())
                sos << " ...\n";
        }));
    }

    void RendererSceneUpdater::activateDisplayContext(DisplayHandle& activeDisplay, DisplayHandle displayToActivate)
    {
        if (activeDisplay != displayToActivate)
        {
            m_renderer.getDisplayController(displayToActivate).getRenderBackend().getSurface().enable();
            activeDisplay = displayToActivate;
        }
    }

    void RendererSceneUpdater::processScreenshotResults()
    {
        for (const auto& resourceManagerIt : m_displayResourceManagers)
        {
            const DisplayHandle display = resourceManagerIt.first;
            const IRendererResourceManager& resourceManager = *resourceManagerIt.second;

            auto screenshots = m_renderer.dispatchProcessedScreenshots(display);

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
                            LOG_INFO(CONTEXT_RENDERER, "RendererSceneUpdater::processScreenshotResults: screenshot file successfully send via dlt: " << screenshot.filename);
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
                    m_rendererEventCollector.addReadPixelsEvent(ERendererEventType_ReadPixelsFromFramebuffer, display, obHandle, std::move(screenshot.pixelData));
                }
            }

        }
    }

}
