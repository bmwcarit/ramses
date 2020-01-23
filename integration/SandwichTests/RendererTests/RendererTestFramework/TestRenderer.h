//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTRENDERER_H
#define RAMSES_TESTRENDERER_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "RendererAPI/Types.h"
#include "RendererAPI/EDeviceTypeId.h"
#include "RendererTestUtils.h"
#include <memory>

namespace ramses
{
    class RamsesFramework;
}

namespace ramses_internal
{
    class IEmbeddedCompositingManager;
    class IEmbeddedCompositor;
    class String;
    class Vector4;

    class TestRenderer
    {
    public:
        virtual ~TestRenderer() {};

        void initializeRendererWithFramework(ramses::RamsesFramework& ramsesFramework, const ramses::RendererConfig& rendererConfig);
        bool isRendererInitialized() const;
        void destroyRendererWithFramework(ramses::RamsesFramework& ramsesFramework);
        void flushRenderer();

        ramses::displayId_t createDisplay(const ramses::DisplayConfig& displayConfig);
        void destroyDisplay              (ramses::displayId_t displayId);
        ramses::displayBufferId_t getDisplayFramebufferId(ramses::displayId_t displayId) const;
        void subscribeScene              (ramses::sceneId_t sceneId, bool blockUntilSubscription = true);
        void hideUnmapAndUnsubscribeScene(ramses::sceneId_t sceneId);
        void unsubscribeScene            (ramses::sceneId_t sceneId);
        void mapScene                    (ramses::displayId_t displayId, ramses::sceneId_t sceneId);
        void unmapScene                  (ramses::sceneId_t sceneId);
        bool showScene                   (ramses::sceneId_t sceneId);
        void hideScene                   (ramses::sceneId_t sceneId);
        void hideAndUnmapScene           (ramses::sceneId_t sceneId);

        void waitForPublication (ramses::sceneId_t sceneId);
        void waitForUnpublished (ramses::sceneId_t sceneId);
        void waitForSubscription(ramses::sceneId_t sceneId);
        void waitForUnmapped    (ramses::sceneId_t sceneId);
        void waitForNamedFlush  (ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag);
        bool waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t streamSource, bool available);
        bool consumeEventsAndCheckExpiredScenes(std::initializer_list<ramses::sceneId_t> sceneIds);
        bool consumeEventsAndCheckRecoveredScenes(std::initializer_list<ramses::sceneId_t> sceneIds);
        void dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler);

        void setLoopMode(ramses::ELoopMode loopMode);
        void startRendererThread();
        void stopRendererThread();
        void doOneLoop();

        ramses::displayBufferId_t createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptible);
        void destroyOffscreenBuffer(ramses::displayId_t displayId, ramses::displayBufferId_t buffer);
        void assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder);
        void setClearColor(ramses::displayId_t displayId, ramses::displayBufferId_t buffer, const ramses_internal::Vector4& clearColor);

        void createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
        void createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId);
        void removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId);

        void updateWarpingMeshData(ramses::displayId_t displayId, const ramses::WarpingMeshData& warpingMeshData);
        bool performScreenshotCheck(const ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const String& comparisonImageFile, float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel, bool readPixelsTwice = false);
        void saveScreenshotForDisplay(const ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const String& imageFile);
        void toggleRendererFrameProfiler();
        IEmbeddedCompositingManager& getEmbeddedCompositorManager(ramses::displayId_t displayId);
        void setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);
        void readPixels(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        IEmbeddedCompositor& getEmbeddedCompositor(ramses::displayId_t displayId);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender);

        bool hasSystemCompositorController() const;

    private:
        ramses::RamsesRenderer* m_renderer = nullptr;
    };
}

#endif
