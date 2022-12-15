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
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "SceneAPI/SceneId.h"
#include "Animation/AnimationSystemFactory.h"
#include "RendererLib/StagingInfo.h"
#include "RendererLib/BufferLinks.h"
#include "RendererLib/FrameTimer.h"
#include "RendererLib/IRendererSceneStateControl.h"
#include "RendererLib/IRendererSceneUpdater.h"
#include "RendererLib/IRendererResourceManager.h"
#include "Scene/EScenePublicationMode.h"
#include "AsyncEffectUploader.h"
#include <unordered_map>

namespace ramses_internal
{
    class RendererCachedScene;
    class Renderer;
    class SceneStateExecutor;
    class IBinaryShaderCache;
    class IRendererResourceCache;
    class RendererEventCollector;
    class RendererScenes;
    class DisplayConfig;
    class SceneExpirationMonitor;
    struct SceneUpdate;
    class DataReferenceLinkManager;
    class TransformationLinkManager;
    class TextureLinkManager;
    class ISceneReferenceLogic;
    class IRenderBackend;
    class IPlatform;
    class IThreadAliveNotifier;

    class RendererSceneUpdater : public IRendererSceneUpdater, public IRendererSceneStateControl
    {
        friend class RendererLogger;
        //TODO (Violin) remove this after KPI Monitor is reworked
        friend class GpuMemorySample;

    public:
        RendererSceneUpdater(
            DisplayHandle display,
            IPlatform& platform,
            Renderer& renderer,
            RendererScenes& rendererScenes,
            SceneStateExecutor& sceneStateExecutor,
            RendererEventCollector& eventCollector,
            FrameTimer& frameTimer,
            SceneExpirationMonitor& expirationMonitor,
            IThreadAliveNotifier& notifier,
            IRendererResourceCache* rendererResourceCache = nullptr);
        virtual ~RendererSceneUpdater() override;

        // IRendererSceneUpdater
        virtual void handleSceneUpdate(SceneId sceneId, SceneUpdate&& sceneUpdate) override;
        virtual void createDisplayContext(const DisplayConfig& displayConfig, IBinaryShaderCache* binaryShaderCache) override;
        virtual void destroyDisplayContext() override;
        virtual void handleScenePublished(SceneId sceneId, EScenePublicationMode mode) override;
        virtual void handleSceneUnpublished(SceneId sceneId) override;
        virtual void handleSceneReceived(const SceneInfo& sceneInfo) override;
        virtual bool handleBufferCreateRequest(OffscreenBufferHandle buffer, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType) override;
        virtual bool handleDmaBufferCreateRequest(OffscreenBufferHandle buffer, UInt32 width, UInt32 height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers) override;
        virtual bool handleBufferDestroyRequest(OffscreenBufferHandle buffer) override;
        virtual bool handleBufferCreateRequest(StreamBufferHandle buffer, WaylandIviSurfaceId source) override;
        virtual bool handleBufferDestroyRequest(StreamBufferHandle buffer) override;
        virtual bool setStreamBufferState(StreamBufferHandle buffer, bool newState) override;
        virtual void handleSetClearFlags(OffscreenBufferHandle buffer, uint32_t clearFlags) override;
        virtual void handleSetClearColor(OffscreenBufferHandle buffer, const Vector4& clearColor) override;
        virtual void handleSetExternallyOwnedWindowSize(uint32_t width, uint32_t height) override;
        virtual void handleReadPixels(OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo) override;
        virtual void handlePickEvent(SceneId sceneId, Vector2 coordsNormalizedToBufferSize) override;
        virtual void handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void handleBufferToSceneDataLinkRequest(StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId) override;
        virtual void setLimitFlushesForceApply(UInt limitForPendingFlushesForceApply) override;
        virtual void setLimitFlushesForceUnsubscribe(UInt limitForPendingFlushesForceUnsubscribe) override;
        virtual void setSkippingOfUnmodifiedScenes(bool enable) override;
        virtual void logRendererInfo(ERendererLogTopic topic, bool verbose, NodeHandle nodeFilter) const override;

        // IRendererSceneStateControl
        virtual void handleSceneSubscriptionRequest     (SceneId sceneId) override;
        virtual void handleSceneUnsubscriptionRequest   (SceneId sceneId, bool indirect) override;
        virtual void handleSceneMappingRequest          (SceneId sceneId) override;
        virtual void handleSceneUnmappingRequest        (SceneId sceneId) override;
        virtual void handleSceneShowRequest             (SceneId sceneId) override;
        virtual void handleSceneHideRequest             (SceneId sceneId) override;
        virtual bool handleSceneDisplayBufferAssignmentRequest(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder) override;

        void updateScenes();

        void processScreenshotResults();
        bool hasPendingFlushes(SceneId sceneId) const;
        void setSceneReferenceLogicHandler(ISceneReferenceLogic& sceneRefLogic);

    protected:
        virtual std::unique_ptr<IRendererResourceManager> createResourceManager(
            IRenderBackend& renderBackend,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            const DisplayConfig& displayConfig,
            IBinaryShaderCache* binaryShaderCache);

        virtual void destroyResourceManager();

        std::chrono::milliseconds m_maximumWaitingTimeToForceMap{ 2000 };

    private:
        void destroyScene(SceneId sceneID);
        void unloadSceneResourcesAndUnrefSceneResources(SceneId sceneId);
        bool markClientAndSceneResourcesForReupload(SceneId sceneId);

        UInt32 updateScenePendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        void applySceneActions(RendererCachedScene& scene, PendingFlush& flushInfo);
        UInt32 applyPendingFlushes(SceneId sceneID, StagingInfo& stagingInfo);
        void processStagedResourceChanges(SceneId sceneID, StagingInfo& stagingInfo);

        bool areResourcesFromPendingFlushesUploaded(SceneId sceneId) const;

        void consolidatePendingSceneActions(SceneId sceneID, SceneUpdate&& sceneUpdate);
        void consolidateResourceDataForMapping(SceneId sceneID);
        void referenceAndProvidePendingResourceData(SceneId sceneID);
        void requestAndUploadAndUnloadResources();
        void uploadUpdatedECStreams();
        void tryToApplyPendingFlushes();
        void processStagedResourceChangesFromAppliedFlushes();
        void handleECStreamAvailabilityChanges();
        void uploadAndUnloadVertexArrays();
        void updateScenesResourceCache();
        void updateScenesRendererAnimations();
        void updateScenesTransformationCache();
        void updateScenesDataLinks();
        void updateScenesStates();

        void resolveDataLinksForConsumerScenes(const DataReferenceLinkManager& dataRefLinkManager);
        void markScenesDependantOnModifiedConsumersAsModified(const DataReferenceLinkManager& dataRefLinkManager, const TransformationLinkManager &transfLinkManager, const TextureLinkManager& texLinkManager);
        void markScenesDependantOnModifiedOffscreenBuffersAsModified(const TextureLinkManager& texLinkManager);

        bool checkIfForceMapNeeded(SceneId sceneId);
        void logTooManyFlushesAndUnsubscribeIfRemoteScene(SceneId sceneId, std::size_t numPendingFlushes);
        void logMissingResources(const PendingData& pendingData, SceneId sceneId) const;
        void logMissingResources(const ResourceContentHashVector& neededResources, SceneId sceneId) const;
        uint32_t getNumberOfPendingNonEmptyFlushes(SceneId sceneId) const;

        DisplayHandle m_display;

        IPlatform&                                        m_platform;
        Renderer&                                         m_renderer;
        RendererScenes&                                   m_rendererScenes;
        SceneStateExecutor&                               m_sceneStateExecutor;
        RendererEventCollector&                           m_rendererEventCollector;
        FrameTimer&                                       m_frameTimer;
        SceneExpirationMonitor&                           m_expirationMonitor;
        ISceneReferenceLogic*                             m_sceneReferenceLogic = nullptr;
        IRendererResourceCache*                           m_rendererResourceCache = nullptr;

        AnimationSystemFactory                            m_animationSystemFactory;

        std::unique_ptr<IRendererResourceManager> m_displayResourceManager;
        std::unique_ptr<AsyncEffectUploader> m_asyncEffectUploader;

        struct SceneMapRequest
        {
            FrameTimer::Clock::time_point requestTimeStamp;
            FrameTimer::Clock::time_point lastLogTimeStamp;
        };
        std::unordered_map<SceneId, SceneMapRequest> m_scenesToBeMapped;

        // extracted from RendererSceneUpdater::updateScenesTransformationCache to avoid per frame allocation
        HashSet<SceneId> m_scenesNeedingTransformationCacheUpdate;

        bool m_skipUnmodifiedScenes = true;
        HashSet<SceneId> m_modifiedScenesToRerender;
        //used as caches for algorithms that mark scenes as modified
        std::vector<SceneId> m_offscreeenBufferModifiedScenesVisitingCache;
        OffscreenBufferLinkVector m_offscreenBufferConsumerSceneLinksCache;

        UInt m_maximumPendingFlushes = 120u;
        UInt m_maximumPendingFlushesToKillScene = 5 * m_maximumPendingFlushes;

        IThreadAliveNotifier& m_notifier;

        // keep as members to avoid runtime re-allocs
        StreamSourceUpdates m_streamUpdates;
        RenderableVector m_tempRenderablesWithUpdatedVertexArrays;
    };
}

#endif
