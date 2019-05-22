//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEUPDATER_H
#define RAMSES_RENDERERSCENEUPDATER_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/DataSlot.h"
#include "Collections/HashMap.h"
#include "Animation/AnimationSystemFactory.h"
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/StagingInfo.h"
#include "RendererLib/OffscreenBufferLinks.h"
#include "RendererLib/FrameTimer.h"
#include "Scene/EScenePublicationMode.h"
#include <unordered_map>

namespace ramses_internal
{
    class IScene;
    class Renderer;
    class SceneStateExecutor;
    class IResourceProvider;
    class IResourceUploader;
    class IRendererResourceCache;
    class IRendererResourceManager;
    class RendererEventCollector;
    class RendererScenes;
    class DisplayConfig;
    class SceneExpirationMonitor;
    class SceneActionCollection;
    class DataReferenceLinkManager;
    class TransformationLinkManager;
    class TextureLinkManager;

    class RendererSceneUpdater
    {
        friend class RendererLogger;
        //TODO (Violin) remove this after KPI Monitor is reworked
        friend class GpuMemorySample;

    public:
        RendererSceneUpdater(Renderer& renderer, RendererScenes& rendererScenes, SceneStateExecutor& sceneStateExecutor, RendererEventCollector& eventCollector, FrameTimer& frameTimer, SceneExpirationMonitor& expirationMonitor, IRendererResourceCache* rendererResourceCache = NULL);
        virtual ~RendererSceneUpdater();

        virtual void handleSceneActions(SceneId sceneId, SceneActionCollection& actionsForScene);

        void createDisplayContext(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle);
        void destroyDisplayContext(DisplayHandle handle);
        void updateScenes();

        void handleScenePublished               (SceneId sceneId, const Guid& clientWhereSceneIsAvailable, EScenePublicationMode mode);
        void handleSceneUnpublished             (SceneId sceneId);
        void handleSceneSubscriptionRequest     (SceneId sceneId);
        void handleSceneUnsubscriptionRequest   (SceneId sceneId, bool indirect);
        void handleSceneMappingRequest          (SceneId sceneId, DisplayHandle handle, Int32 sceneRenderOrder);
        void handleSceneUnmappingRequest        (SceneId sceneId);
        void handleSceneShowRequest             (SceneId sceneId);
        void handleSceneHideRequest             (SceneId sceneId);
        void handleSceneReceived                (const SceneInfo& sceneInfo);
        Bool handleBufferCreateRequest          (OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, Bool isDoubleBuffered);
        Bool handleBufferDestroyRequest         (OffscreenBufferHandle buffer, DisplayHandle display);
        Bool handleSceneOffscreenBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer);
        Bool handleSceneFramebufferAssignmentRequest    (SceneId sceneId);
        void handleSceneDataLinkRequest         (SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId);
        void handleBufferToSceneDataLinkRequest (OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId);
        void handleDataUnlinkRequest            (SceneId consumerSceneId, DataSlotId consumerId);

        Bool hasPendingFlushes(SceneId sceneId) const;

        const HashSet<SceneId>& getModifiedScenes() const;

        void setLimitFlushesForceApply(UInt limitForPendingFlushesForceApply);
        void setLimitFlushesForceUnsubscribe(UInt limitForPendingFlushesForceUnsubscribe);

        static constexpr UInt SceneActionsPerChunkToApply = 100u;

    private:
        void destroyScene(SceneId sceneID);
        void unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId);
        bool markClientAndSceneResourcesForReupload(SceneId sceneId);
        void appendPendingSceneActions(SceneId sceneId, SceneActionCollection& actionsForScene);

        UInt32 updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        void applySceneActions(IScene& scene, PendingFlush& flushInfo);
        void applySceneActionsPartially(IScene& scene, PendingFlush& flushInfo, bool firstChunk);
        UInt32 applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo, EResourceStatus resourcesStatus, Bool applyFlushPartially);
        void processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo, DisplayHandle& activeDisplay);

        Bool willApplyingChangesMakeAllResourcesAvailable(SceneId sceneId) const;
        Bool areClientResourcesInUseUploaded(SceneId sceneId) const;

        void consolidatePendingSceneActions();
        void consolidatePendingSceneActions(SceneId sceneID, SceneActionCollection& actionsForScene);
        void consolidateResourceChanges(PendingFlush& flushInfo, const PendingFlushes& pendingFlushes, const SceneResourceChanges& resourceChanges, ResourceContentHashVector& newlyNeededClientResources) const;
        void requestAndUploadAndUnloadResources(DisplayHandle& activeDisplay);
        void updateEmbeddedCompositingResources(DisplayHandle& activeDisplay);
        void tryToApplyPendingFlushes();
        void processStagedResourceChangesFromAppliedFlushes(DisplayHandle& activeDisplay);
        void updateSceneStreamTexturesDirtiness();
        void updateScenesResourceCache();
        void updateScenesRealTimeAnimationSystems();
        void updateScenesTransformationCache();
        void updateScenesDataLinks();
        void updateScenesStates();

        void activateDisplayContext(DisplayHandle& activeDisplay, DisplayHandle displayToActivate);

        void resolveDataLinksForConsumerScenes(const DataReferenceLinkManager& dataRefLinkManager);
        void markScenesDependantOnModifiedConsumersAsModified(const DataReferenceLinkManager& dataRefLinkManager, const TransformationLinkManager &transfLinkManager, const TextureLinkManager& texLinkManager);
        void markScenesDependantOnModifiedOffscreenBuffersAsModified(const TextureLinkManager& texLinkManager);

        void logMissingResources(const ResourceContentHashVector& resourceVector, SceneId sceneId) const;

        Renderer&                                         m_renderer;
        RendererScenes&                                   m_rendererScenes;
        SceneStateExecutor&                               m_sceneStateExecutor;
        RendererEventCollector&                           m_rendererEventCollector;
        FrameTimer&                                       m_frameTimer;
        SceneExpirationMonitor&                           m_expirationMonitor;
        IRendererResourceCache*                           m_rendererResourceCache;

        AnimationSystemFactory                            m_animationSystemFactory;

        HashMap<DisplayHandle, IRendererResourceManager*> m_displayResourceManagers;

        std::unordered_map<SceneId, std::vector<SceneActionCollection>> m_pendingSceneActions;

        struct SceneMapRequest
        {
            DisplayHandle display;
            Int32 sceneRenderOrder;
            FrameTimer::Clock::time_point requestTimeStamp;
            FrameTimer::Clock::time_point lastLogTimeStamp;
        };
        typedef HashMap<SceneId, SceneMapRequest> SceneMapRequests;
        SceneMapRequests m_scenesToBeMapped;

        // extracted from RendererSceneUpdater::updateScenesTransformationCache to avoid per frame allocation
        HashSet<SceneId> m_scenesNeedingTransformationCacheUpdate;

        HashSet<SceneId> m_modifiedScenesToRerender;
        //used as caches for algorithms that mark scenes as modified
        std::vector<SceneId> m_offscreeenBufferModifiedScenesVisitingCache;
        OffscreenBufferLinkVector m_offscreenBufferConsumerSceneLinksCache;

        UInt m_maximumPendingFlushes = 60u;
        UInt m_maximumPendingFlushesToKillScene = 5 * 60u;
    };
}

#endif
