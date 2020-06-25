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
#include "Collections/HashMap.h"
#include "Animation/AnimationSystemFactory.h"
#include "RendererLib/EResourceStatus.h"
#include "RendererLib/StagingInfo.h"
#include "RendererLib/OffscreenBufferLinks.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/IRendererSceneControl.h"
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
    class ISceneReferenceLogic;

    class RendererSceneUpdater : public IRendererSceneControl
    {
        friend class RendererLogger;
        //TODO (Violin) remove this after KPI Monitor is reworked
        friend class GpuMemorySample;

    public:
        RendererSceneUpdater(
            Renderer& renderer,
            RendererScenes& rendererScenes,
            SceneStateExecutor& sceneStateExecutor,
            RendererEventCollector& eventCollector,
            FrameTimer& frameTimer,
            SceneExpirationMonitor& expirationMonitor,
            IRendererResourceCache* rendererResourceCache = nullptr);
        virtual ~RendererSceneUpdater();

        virtual void handleSceneActions(SceneId sceneId, SceneActionCollection&& actionsForScene);

        void createDisplayContext(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle);
        void destroyDisplayContext(DisplayHandle handle);
        void updateScenes();

        // IRendererSceneControl
        virtual void handleSceneSubscriptionRequest     (SceneId sceneId) override;
        virtual void handleSceneUnsubscriptionRequest   (SceneId sceneId, bool indirect) override;
        virtual void handleSceneMappingRequest          (SceneId sceneId, DisplayHandle handle) override;
        virtual void handleSceneUnmappingRequest        (SceneId sceneId) override;
        virtual void handleSceneShowRequest             (SceneId sceneId) override;
        virtual void handleSceneHideRequest             (SceneId sceneId) override;
        virtual bool handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder) override;
        virtual void handleSceneDataLinkRequest         (SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void handleBufferToSceneDataLinkRequest (OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void handleDataUnlinkRequest            (SceneId consumerSceneId, DataSlotId consumerId) override;

        void handleScenePublished               (SceneId sceneId, EScenePublicationMode mode);
        void handleSceneUnpublished             (SceneId sceneId);
        void handleSceneReceived                (const SceneInfo& sceneInfo);
        Bool handleBufferCreateRequest          (OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, Bool isDoubleBuffered);
        Bool handleBufferDestroyRequest         (OffscreenBufferHandle buffer, DisplayHandle display);
        void handleSetClearColor                (DisplayHandle display, OffscreenBufferHandle buffer, const Vector4& clearColor);
        virtual void handlePickEvent            (SceneId sceneId, Vector2 coordsNormalizedToBufferSize);

        Bool hasPendingFlushes(SceneId sceneId) const;

        void setLimitFlushesForceApply(UInt limitForPendingFlushesForceApply);
        void setLimitFlushesForceUnsubscribe(UInt limitForPendingFlushesForceUnsubscribe);

        void setSceneReferenceLogicHandler(ISceneReferenceLogic& sceneRefLogic);

    private:
        void destroyScene(SceneId sceneID);
        void unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId);
        bool markClientAndSceneResourcesForReupload(SceneId sceneId);

        UInt32 updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        void applySceneActions(IScene& scene, PendingFlush& flushInfo);
        UInt32 applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo, EResourceStatus resourcesStatus);
        void processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo, DisplayHandle& activeDisplay);

        Bool willApplyingChangesMakeAllResourcesAvailable(SceneId sceneId) const;
        Bool areClientResourcesInUseUploaded(SceneId sceneId) const;

        void logTooManyFlushesAndUnsubscribeIfRemoteScene(SceneId sceneId, std::size_t numPendingFlushes);
        void consolidatePendingSceneActions(SceneId sceneID, SceneActionCollection&& actionsForScene);
        void consolidateResourceChanges(PendingData& pendingData, const SceneResourceChanges& resourceChanges, ResourceContentHashVector& newlyNeededClientResources) const;
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
        ISceneReferenceLogic*                             m_sceneReferenceLogic = nullptr;
        IRendererResourceCache*                           m_rendererResourceCache = nullptr;

        AnimationSystemFactory                            m_animationSystemFactory;

        HashMap<DisplayHandle, IRendererResourceManager*> m_displayResourceManagers;

        struct SceneMapRequest
        {
            DisplayHandle display;
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
