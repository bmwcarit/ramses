//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERTESTSFRAMEWORK_H
#define RAMSES_RENDERERTESTSFRAMEWORK_H

#include "RendererTestInstance.h"
#include "RenderingTestCase.h"
#include "Collections/Pair.h"
#include "Utils/StringUtils.h"
#include "Math3d/Vector3.h"
#include "RendererTestUtils.h"

class IRendererTest;

typedef std::vector<RenderingTestCase*> RenderingTestCases;

namespace ramses
{
    class WarpingMeshData;
}

class RendererTestsFramework
{
public:
    explicit RendererTestsFramework(bool generateScreenshots, const ramses::RamsesFrameworkConfig& config);
    ~RendererTestsFramework();

    void initializeRenderer();
    void initializeRenderer(const ramses::RendererConfig& rendererConfig);
    void destroyRenderer();
    ramses::displayId_t createDisplay(ramses::DisplayConfig displayConfig);
    void destroyDisplays();
    RendererTestInstance& getTestRenderer();
    TestScenes& getScenesRegistry();
    ramses::RamsesClient& getClient();

    RenderingTestCase& createTestCase(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name);
    RenderingTestCase& createTestCaseWithDefaultDisplay(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name, bool iviWindowStartVisible = true);
    RenderingTestCase& createTestCaseWithoutRenderer(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name);

    void subscribeScene(ramses::sceneId_t sceneId);
    void unsubscribeScene(ramses::sceneId_t sceneId);
    void mapScene(ramses::sceneId_t sceneId, uint32_t testDisplayIdx = 0u, ramses_internal::Int32 sceneRenderOrder = 0);
    void unmapScene(ramses::sceneId_t sceneId);
    void showScene(ramses::sceneId_t sceneId);
    void hideScene(ramses::sceneId_t sceneId);
    void hideAndUnmap(ramses::sceneId_t sceneId);
    void dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler);
    ramses::offscreenBufferId_t createOffscreenBuffer       (uint32_t testDisplayIdx, uint32_t width, uint32_t height, bool interruptible);
    void                        destroyOffscreenBuffer      (uint32_t testDisplayIdx, ramses::offscreenBufferId_t buffer);
    void                        assignSceneToOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t buffer);
    void                        assignSceneToFramebuffer    (ramses::sceneId_t sceneId);
    void createBufferDataLink(ramses::offscreenBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerTag, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag);
    void setWarpingMeshData(const ramses::WarpingMeshData& meshData, uint32_t testDisplayIdx = 0u);
    void publishAndFlushScene(ramses::sceneId_t sceneId);
    void flushRendererAndDoOneLoop();
    bool renderAndCompareScreenshot(const ramses_internal::String& expectedImageName, uint32_t testDisplayIdx = 0u, float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel);
    bool renderAndCompareScreenshotSubimage(const ramses_internal::String& expectedImageName, ramses_internal::UInt32 subimageX, ramses_internal::UInt32 subimageY, ramses_internal::UInt32 subimageWidth, ramses_internal::UInt32 subimageHeight, float maxAveragePercentErrorPerPixel = RendererTestUtils::DefaultMaxAveragePercentPerPixel);
    void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender);
    void filterTestCases(const ramses_internal::StringVector& filterIn, const ramses_internal::StringVector& filterOut);

    bool runAllTests();
    ramses_internal::String generateReport() const;

    static bool NameMatchesFilter(const ramses_internal::String& name, const ramses_internal::StringVector& filter);

    template <typename INTEGRATION_SCENE>
    ramses::sceneId_t createAndShowScene(ramses_internal::UInt32 sceneState, const ramses_internal::Vector3& cameraPosition = ramses_internal::Vector3(0.0f), int32_t sceneRenderOrder = 0, const ramses::SceneConfig& sceneConfig = ramses::SceneConfig())
    {
        const ramses::sceneId_t sceneId = getScenesRegistry().createScene<INTEGRATION_SCENE>(sceneState, cameraPosition, sceneConfig);
        publishAndFlushScene(sceneId);
        subscribeScene(sceneId);
        mapScene(sceneId, 0, sceneRenderOrder);
        showScene(sceneId);
        return sceneId;
    }

protected:
    typedef std::vector<ramses::offscreenBufferId_t> OffscreenBufferVector;
    struct TestDisplayInfo
        {
            ramses::displayId_t displayId;
            ramses::DisplayConfig config;
            OffscreenBufferVector offscreenBuffers;
        };

    typedef std::vector<TestDisplayInfo> TestDisplays;
    const TestDisplays& getDisplays() const;

private:
    bool compareScreenshotInternal(
        const ramses_internal::String& expectedImageName,
        ramses::displayId_t displayId,
        float maxAveragePercentErrorPerPixel,
        ramses_internal::UInt32 subimageX,
        ramses_internal::UInt32 subimageY,
        ramses_internal::UInt32 subimageWidth,
        ramses_internal::UInt32 subimageHeight);

    void sortTestCases();
    bool currentDisplaySetupMatchesTestCase(const RenderingTestCase& testCase) const;
    bool applyRendererAndDisplaysConfigurationForTest(const RenderingTestCase& testCase);
    void destroyScenes();
    void destroyOffscreenBuffers();
    bool runTestCase(RenderingTestCase& testCase);

    const bool                      m_generateScreenshots;
    RendererTestInstance            m_testRenderer;
    TestDisplays                    m_displays;

    RenderingTestCases              m_testCases;

    // for logging and debugging purpose
    RenderingTestCase*              m_activeTestCase;
    RenderingTestCases              m_passedTestCases;
    RenderingTestCases              m_failedTestCases;
    ramses_internal::UInt64         m_elapsedTime;
};

#endif
