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
#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/RendererSceneState.h"
#include "RendererAPI/Types.h"
#include "RendererTestUtils.h"

#include <memory>
#include <string>

namespace ramses
{
    class RamsesFramework;
    class IRendererEventHandler;
    class IRendererSceneControlEventHandler;
}

namespace ramses_internal
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

        ramses::displayId_t createDisplay(const ramses::DisplayConfig& displayConfig);
        void destroyDisplay(ramses::displayId_t displayId);
        [[nodiscard]] ramses::displayBufferId_t getDisplayFramebufferId(ramses::displayId_t displayId) const;

        void setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t display);
        bool getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        void setSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        bool waitForSceneStateChange(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        void waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag);
        bool checkScenesExpired(std::initializer_list<ramses::sceneId_t> sceneIds);
        bool checkScenesNotExpired(std::initializer_list<ramses::sceneId_t> sceneIds);
        bool waitForStreamSurfaceAvailabilityChange(ramses::waylandIviSurfaceId_t streamSource, bool available);

        void dispatchEvents(ramses::IRendererEventHandler& eventHandler, ramses::IRendererSceneControlEventHandler& sceneControlEventHandler);

        void setLoopMode(ramses::ELoopMode loopMode);
        void startRendererThread();
        void stopRendererThread();
        [[nodiscard]] bool isRendererThreadEnabled() const;
        void doOneLoop();

        ramses::displayBufferId_t createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptible, uint32_t sampleCount = 0u, ramses::EDepthBufferType depthBufferType = ramses::EDepthBufferType::DepthStencil);
        ramses::displayBufferId_t createDmaOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t bufferUsageFlags, uint64_t modifier);
        void destroyOffscreenBuffer(ramses::displayId_t displayId, ramses::displayBufferId_t buffer);
        void assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder);
        void setClearFlags(ramses::displayId_t displayId, ramses::displayBufferId_t buffer, uint32_t clearFlags);
        void setClearColor(ramses::displayId_t displayId, ramses::displayBufferId_t buffer, const glm::vec4& clearColor);
        bool getDmaOffscreenBufferFDAndStride(ramses::displayId_t displayId, ramses::displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;

        ramses::streamBufferId_t createStreamBuffer(ramses::displayId_t displayId, ramses::waylandIviSurfaceId_t source);
        void destroyStreamBuffer(ramses::displayId_t displayId, ramses::streamBufferId_t buffer);

        void createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
        void createBufferDataLink(ramses::streamBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
        void createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId);
        void removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId);

        bool performScreenshotCheck(
            const ramses::displayId_t displayId,
            ramses::displayBufferId_t bufferId,
            uint32_t x, uint32_t y, uint32_t width, uint32_t height,
            const std::string& comparisonImageFile,
            float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel,
            bool readPixelsTwice = false,
            bool saveDiffOnError = true);
        void saveScreenshotForDisplay(const ramses::displayId_t displayId, ramses::displayBufferId_t bufferId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::string& imageFile);
        void readPixels(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        void setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility);

        IEmbeddedCompositor& getEmbeddedCompositor(ramses::displayId_t displayId);
        IEmbeddedCompositingManager& getEmbeddedCompositorManager(ramses::displayId_t displayId);

        [[nodiscard]] bool hasSystemCompositorController() const;

    private:
        ramses::RamsesRenderer* m_renderer = nullptr;
        ramses::RendererSceneControl* m_sceneControlAPI = nullptr;
    };
}

#endif
