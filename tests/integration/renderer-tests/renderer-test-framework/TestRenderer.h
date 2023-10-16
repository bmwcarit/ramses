//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/Types.h"
#include "ramses/framework/RendererSceneState.h"
#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "RendererTestUtils.h"

#include <memory>
#include <string>

namespace ramses
{
    class RamsesFramework;
    class Scene;
    class IRendererEventHandler;
    class IRendererSceneControlEventHandler;
}

namespace ramses::internal
{
    class IEmbeddedCompositingManager;
    class IEmbeddedCompositor;

    class TestRenderer
    {
    public:
        virtual ~TestRenderer() = default;

        void initializeRendererWithFramework(ramses::RamsesFramework& ramsesFramework, const ramses::RendererConfig& rendererConfig);
        [[nodiscard]] bool isRendererInitialized() const;
        void destroyRendererWithFramework(ramses::RamsesFramework& ramsesFramework);
        void flushRenderer();

        displayId_t createDisplay(const ramses::DisplayConfig& displayConfig);
        void destroyDisplay(displayId_t displayId);
        [[nodiscard]] displayBufferId_t getDisplayFramebufferId(displayId_t displayId) const;

        void setSceneMapping(sceneId_t sceneId, displayId_t display);
        bool getSceneToState(ramses::Scene& scene, ramses::RendererSceneState state);
        void setSceneState(sceneId_t sceneId, ramses::RendererSceneState state);
        bool waitForSceneStateChange(ramses::Scene& scene, ramses::RendererSceneState state);
        void waitForFlush(sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag);
        bool checkScenesExpired(std::initializer_list<sceneId_t> sceneIds);
        bool checkScenesNotExpired(std::initializer_list<sceneId_t> sceneIds);
        bool waitForStreamSurfaceAvailabilityChange(waylandIviSurfaceId_t streamSource, bool available);

        void dispatchEvents(IRendererEventHandler& eventHandler, IRendererSceneControlEventHandler& sceneControlEventHandler);

        void setLoopMode(ELoopMode loopMode);
        void startRendererThread();
        void stopRendererThread();
        [[nodiscard]] bool isRendererThreadEnabled() const;
        void doOneLoop();

        displayBufferId_t createOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, bool interruptible, uint32_t sampleCount = 0u, EDepthBufferType depthBufferType = EDepthBufferType::DepthStencil);
        displayBufferId_t createDmaOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t bufferUsageFlags, uint64_t modifier);
        void destroyOffscreenBuffer(displayId_t displayId, displayBufferId_t buffer);
        void assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t buffer, int32_t renderOrder);
        void setClearFlags(displayId_t displayId, displayBufferId_t buffer, ClearFlags clearFlags);
        void setClearColor(displayId_t displayId, displayBufferId_t buffer, const glm::vec4& clearColor);
        bool getDmaOffscreenBufferFDAndStride(displayId_t displayId, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;

        streamBufferId_t createStreamBuffer(displayId_t displayId, waylandIviSurfaceId_t source);
        void destroyStreamBuffer(displayId_t displayId, streamBufferId_t buffer);

        void createBufferDataLink(displayBufferId_t providerBuffer, sceneId_t consumerScene, dataConsumerId_t consumerTag);
        void createBufferDataLink(streamBufferId_t providerBuffer, sceneId_t consumerScene, dataConsumerId_t consumerTag);
        void createDataLink(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId);
        void removeDataLink(sceneId_t consumerScene, dataConsumerId_t consumerId);

        bool performScreenshotCheck(
            const displayId_t displayId,
            displayBufferId_t bufferId,
            uint32_t x, uint32_t y, uint32_t width, uint32_t height,
            const std::string& comparisonImageFile,
            float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel,
            bool readPixelsTwice = false,
            bool saveDiffOnError = true);
        void saveScreenshotForDisplay(const displayId_t displayId, displayBufferId_t bufferId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::string& imageFile);
        void readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        void setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);

        IEmbeddedCompositor& getEmbeddedCompositor(displayId_t displayId);
        IEmbeddedCompositingManager& getEmbeddedCompositorManager(displayId_t displayId);

        [[nodiscard]] bool hasSystemCompositorController() const;

    private:
        RamsesRenderer* m_renderer = nullptr;
        RendererSceneControl* m_sceneControlAPI = nullptr;
    };
}
