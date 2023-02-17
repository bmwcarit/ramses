//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERTESTSFRAMEWORK_H
#define RAMSES_RENDERERTESTSFRAMEWORK_H

#include "TestScenesAndRenderer.h"
#include "RenderingTestCase.h"
#include "Collections/Pair.h"
#include "Utils/StringUtils.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "RendererTestUtils.h"

class IRendererTest;

using RenderingTestCases = std::vector<RenderingTestCase *>;

namespace ramses
{
    class IRendererEventHandler;
    class IRendererSceneControlEventHandler;
}

class RendererTestsFramework
{
public:
    explicit RendererTestsFramework(bool generateScreenshots, const ramses::RamsesFrameworkConfig& config);
    ~RendererTestsFramework();

    void initializeRenderer();
    void initializeRenderer(const ramses::RendererConfig& rendererConfig);
    void destroyRenderer();
    ramses::displayId_t createDisplay(const ramses::DisplayConfig& displayConfig);
    [[nodiscard]] ramses::displayBufferId_t getDisplayFramebufferId(uint32_t testDisplayIdx) const;
    void destroyDisplays();
    ramses_internal::TestRenderer& getTestRenderer();
    TestScenes& getScenesRegistry();
    ramses::RamsesClient& getClient();

    RenderingTestCase& createTestCase(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name);
    RenderingTestCase& createTestCaseWithDefaultDisplay(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name, bool iviWindowStartVisible = true);
    RenderingTestCase& createTestCaseWithoutRenderer(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name);

    void setSceneMapping(ramses::sceneId_t sceneId, uint32_t testDisplayIdx);
    bool getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
    bool getSceneToRendered(ramses::sceneId_t sceneId, uint32_t testDisplayIdx = 0);

    void dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler, ramses::IRendererSceneControlEventHandler& sceneControlEventHandler);
    ramses::displayBufferId_t   createOffscreenBuffer(uint32_t testDisplayIdx, uint32_t width, uint32_t height, bool interruptible, uint32_t sampleCount = 0u, ramses::EDepthBufferType depthBufferType = ramses::EDepthBufferType_DepthStencil);
    ramses::displayBufferId_t   createDmaOffscreenBuffer(uint32_t testDisplayIdx, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t bufferUsageFlags, uint64_t modifier);
    bool                        getDmaOffscreenBufferFDAndStride(uint32_t testDisplayIdx, ramses::displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;
    void                        destroyOffscreenBuffer(uint32_t testDisplayIdx, ramses::displayBufferId_t buffer);
    ramses::streamBufferId_t    createStreamBuffer(uint32_t testDisplayIdx, ramses::waylandIviSurfaceId_t source);
    void                        destroyStreamBuffer(uint32_t testDisplayIdx, ramses::streamBufferId_t buffer);
    void assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder = 0);
    void createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void createBufferDataLink(ramses::streamBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerTag, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void setClearFlags(uint32_t testDisplayIdx, ramses::displayBufferId_t ob, uint32_t clearFlags);
    void setClearColor(uint32_t testDisplayIdx, ramses::displayBufferId_t ob, const ramses_internal::Vector4& clearColor);
    void publishAndFlushScene(ramses::sceneId_t sceneId);
    void flushRendererAndDoOneLoop();
    bool renderAndCompareScreenshot(
        const ramses_internal::String& expectedImageName,
        uint32_t testDisplayIdx = 0u,
        float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel,
        bool readPixelsTwice = false,
        bool saveDiffOnError = true);
    bool renderAndCompareScreenshotOffscreenBuffer(const ramses_internal::String& expectedImageName,
                                                   uint32_t testDisplayIdx, ramses::displayBufferId_t displayBuffer, uint32_t width, uint32_t height,
                                                   float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel);
    bool renderAndCompareScreenshotSubimage(const ramses_internal::String& expectedImageName,
                                            ramses_internal::UInt32 subimageX, ramses_internal::UInt32 subimageY, ramses_internal::UInt32 subimageWidth, ramses_internal::UInt32 subimageHeight,
                                            float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel, bool readPixelsTwice = false);
    void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
    void filterTestCases(const ramses_internal::StringVector& filterIn, const ramses_internal::StringVector& filterOut);

    bool runAllTests();
    [[nodiscard]] std::string generateReport() const;

    static bool NameMatchesFilter(const ramses_internal::String& name, const ramses_internal::StringVector& filter);

    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createAndShowScene(
        uint32_t sceneState,
        const ramses_internal::Vector3& cameraPosition = ramses_internal::Vector3(0.0f),
        const ramses::SceneConfig& sceneConfig = {})
    {
        const ramses::sceneId_t sceneId = getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, cameraPosition, sceneConfig);
        publishAndFlushScene(sceneId);
        getSceneToRendered(sceneId);
        return sceneId;
    }

    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createAndShowScene(uint32_t sceneState, uint32_t vpWidth, uint32_t vpHeight)
    {
        const ramses::sceneId_t sceneId = getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, { 0, 0, 0 }, vpWidth, vpHeight);
        publishAndFlushScene(sceneId);
        getSceneToRendered(sceneId);
        return sceneId;
    }

protected:
    struct TestDisplayInfo
    {
        ramses::displayId_t displayId;
        ramses::DisplayConfig config;
        std::vector<ramses::displayBufferId_t> offscreenBuffers;
        std::vector<ramses::streamBufferId_t> streamBuffers;
    };

    using TestDisplays = std::vector<TestDisplayInfo>;
    [[nodiscard]] const TestDisplays& getDisplays() const;

private:
    bool compareScreenshotInternal(
        const ramses_internal::String& expectedImageName,
        ramses::displayId_t displayId,
        ramses::displayBufferId_t bufferId,
        float maxAveragePercentErrorPerPixel,
        ramses_internal::UInt32 subimageX,
        ramses_internal::UInt32 subimageY,
        ramses_internal::UInt32 subimageWidth,
        ramses_internal::UInt32 subimageHeight,
        bool readPixelsTwice,
        bool saveDiffOnError);

    void sortTestCases();
    [[nodiscard]] bool currentDisplaySetupMatchesTestCase(const RenderingTestCase& testCase) const;
    bool applyRendererAndDisplaysConfigurationForTest(const RenderingTestCase& testCase);
    void destroyScenes();
    void destroyBuffers();
    bool runTestCase(RenderingTestCase& testCase);

    const bool                      m_generateScreenshots;
    ramses_internal::TestScenesAndRenderer m_testScenesAndRenderer;
    ramses_internal::TestRenderer&  m_testRenderer;
    TestDisplays                    m_displays;
    bool                            m_forceDisplaysReinitForNextTestCase = false;

    RenderingTestCases              m_testCases;

    // for logging and debugging purpose
    RenderingTestCase*              m_activeTestCase;
    RenderingTestCases              m_passedTestCases;
    RenderingTestCases              m_failedTestCases;
    ramses_internal::UInt64         m_elapsedTime;
};

#endif
