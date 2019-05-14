//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererTestsFramework.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "DisplayConfigImpl.h"
#include "IRendererTest.h"
#include "DisplayConfigImpl.h"
#include "RendererAPI/IDisplayController.h"
#include "RendererAPI/IDevice.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "RendererTestUtils.h"
#include "RendererTestEventHandler.h"

RendererTestsFramework::RendererTestsFramework(bool generateScreenshots, const ramses::RamsesFrameworkConfig& config)
    : m_generateScreenshots(generateScreenshots)
    , m_testRenderer(config)
    , m_activeTestCase(NULL)
    , m_elapsedTime(0u)
{
}

RendererTestsFramework::~RendererTestsFramework()
{
    assert(m_activeTestCase == NULL);
    for(const auto& testCase : m_testCases)
    {
        delete testCase;
    }

    destroyDisplays();
    m_testRenderer.destroyRenderer();
}

void RendererTestsFramework::initializeRenderer()
{
    m_testRenderer.initializeRenderer();
}

void RendererTestsFramework::initializeRenderer(const ramses::RendererConfig& rendererConfig)
{
    m_testRenderer.initializeRenderer(rendererConfig);
}

void RendererTestsFramework::destroyRenderer()
{
    m_testRenderer.destroyRenderer();
}

ramses::displayId_t RendererTestsFramework::createDisplay(ramses::DisplayConfig displayConfig)
{
    const ramses::displayId_t displayId = m_testRenderer.createDisplay(displayConfig);
    if (displayId != ramses::InvalidDisplayId)
    {
        m_displays.push_back({displayId, displayConfig, OffscreenBufferVector()});
    }

    return displayId;
}

RendererTestInstance& RendererTestsFramework::getTestRenderer()
{
    return m_testRenderer;
}

RenderingTestCase& RendererTestsFramework::createTestCase(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name)
{
    RenderingTestCase* testCase = new RenderingTestCase(id, rendererTest, name, true);
    m_testCases.push_back(testCase);

    return *testCase;
}

RenderingTestCase& RendererTestsFramework::createTestCaseWithDefaultDisplay(ramses_internal::UInt32 id, IRendererTest& rendererTest, const ramses_internal::String& name, bool iviWindowStartVisible)
{
    RenderingTestCase& testCase = createTestCase(id, rendererTest, name);
    ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, iviWindowStartVisible);
    displayConfig.setWindowRectangle(0, 0, ramses_internal::IntegrationScene::DefaultDisplayWidth, ramses_internal::IntegrationScene::DefaultDisplayHeight);
    displayConfig.setPerspectiveProjection(19.f,
        static_cast<float>(ramses_internal::IntegrationScene::DefaultDisplayWidth) / ramses_internal::IntegrationScene::DefaultDisplayHeight,
        0.1f, 1500.f);
    testCase.m_displayConfigs.push_back(displayConfig);

    return testCase;
}

RenderingTestCase& RendererTestsFramework::createTestCaseWithoutRenderer(ramses_internal::UInt32 id, IRendererTest &rendererTest, const ramses_internal::String &name)
{
    RenderingTestCase* testCase = new RenderingTestCase(id, rendererTest, name, false);
    m_testCases.push_back(testCase);

    return *testCase;
}

TestScenes& RendererTestsFramework::getScenesRegistry()
{
    return m_testRenderer.getScenesRegistry();
}

ramses::RamsesClient& RendererTestsFramework::getClient()
{
    return m_testRenderer.getClient();
}

void RendererTestsFramework::subscribeScene(ramses::sceneId_t sceneId)
{
    // subscription consists of two steps - sending a request and receiving a scene
    // subscription is followed by scene map in most tests
    // map command execution would fail if scene was not received
    m_testRenderer.subscribeScene(sceneId);
}

void RendererTestsFramework::unsubscribeScene(ramses::sceneId_t sceneId)
{
    m_testRenderer.unsubscribeScene(sceneId);
}

void RendererTestsFramework::mapScene(ramses::sceneId_t sceneId, uint32_t testDisplayIdx, ramses_internal::Int32 sceneRenderOrder)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;

    // mapping consists of two steps - sending a request and uploading resources of the scene
    // mapping is followed by scene show in most tests
    // show command execution would fail if scene was not mapped before
    // Ensure scene is in mapped state after this function has ended
    m_testRenderer.mapScene(displayId, sceneId, sceneRenderOrder);
}

void RendererTestsFramework::unmapScene(ramses::sceneId_t sceneId)
{
    m_testRenderer.unmapScene(sceneId);
}

void RendererTestsFramework::showScene(ramses::sceneId_t sceneId)
{
    m_testRenderer.showScene(sceneId);
}

void RendererTestsFramework::hideScene(ramses::sceneId_t sceneId)
{
    m_testRenderer.hideScene(sceneId);
}

void RendererTestsFramework::hideAndUnmap(ramses::sceneId_t sceneId)
{
    m_testRenderer.hideAndUnmapScene(sceneId);
}

void RendererTestsFramework::dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler)
{
    m_testRenderer.dispatchRendererEvents(eventHandler);
}

ramses::offscreenBufferId_t RendererTestsFramework::createOffscreenBuffer(uint32_t testDisplayIdx, uint32_t width, uint32_t height, bool interruptible)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    ramses::offscreenBufferId_t buffer = m_testRenderer.createOffscreenBuffer(displayId, width, height, interruptible);
    m_displays[testDisplayIdx].offscreenBuffers.push_back(buffer);
    return buffer;
}

void RendererTestsFramework::destroyOffscreenBuffer(uint32_t testDisplayIdx, ramses::offscreenBufferId_t buffer)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    OffscreenBufferVector& offscreenBuffers = m_displays[testDisplayIdx].offscreenBuffers;

    OffscreenBufferVector::iterator bufferIter = ramses_internal::find_c(offscreenBuffers, buffer);
    assert(bufferIter != offscreenBuffers.end());
    offscreenBuffers.erase(bufferIter);

    m_testRenderer.destroyOffscreenBuffer(displayId, buffer);
}

void RendererTestsFramework::assignSceneToOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t buffer)
{
    m_testRenderer.assignSceneToOffscreenBuffer(sceneId, buffer);
}

void RendererTestsFramework::assignSceneToFramebuffer(ramses::sceneId_t sceneId)
{
    m_testRenderer.assignSceneToFramebuffer(sceneId);
}

void RendererTestsFramework::createBufferDataLink(ramses::offscreenBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    m_testRenderer.createBufferDataLink(providerBuffer, consumerScene, consumerTag);
}

void RendererTestsFramework::createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
{
    m_testRenderer.createDataLink(providerScene, providerId, consumerScene, consumerId);
}

void RendererTestsFramework::removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
{
    m_testRenderer.removeDataLink(consumerScene, consumerId);
}

void RendererTestsFramework::setWarpingMeshData(const ramses::WarpingMeshData& meshData, uint32_t testDisplayIdx)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    m_testRenderer.updateWarpingMeshData(displayId, meshData);
}

void RendererTestsFramework::publishAndFlushScene(ramses::sceneId_t sceneId)
{
    m_testRenderer.flush(sceneId);
    m_testRenderer.publish(sceneId);
}

void RendererTestsFramework::flushRendererAndDoOneLoop()
{
    m_testRenderer.flushRenderer();
    m_testRenderer.doOneLoop();
}

bool RendererTestsFramework::renderAndCompareScreenshot(const ramses_internal::String& expectedImageName, uint32_t testDisplayIdx, float maxAveragePercentErrorPerPixel)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    const ramses::DisplayConfig& displayConfig = m_displays[testDisplayIdx].config;

    return compareScreenshotInternal(
        expectedImageName,
        displayId,
        maxAveragePercentErrorPerPixel,
        0u,
        0u,
        displayConfig.impl.getInternalDisplayConfig().getDesiredWindowWidth(),
        displayConfig.impl.getInternalDisplayConfig().getDesiredWindowHeight());
}

bool RendererTestsFramework::renderAndCompareScreenshotSubimage(const ramses_internal::String& expectedImageName, ramses_internal::UInt32 subimageX, ramses_internal::UInt32 subimageY, ramses_internal::UInt32 subimageWidth, ramses_internal::UInt32 subimageHeight, float maxAveragePercentErrorPerPixel)
{
    return compareScreenshotInternal(expectedImageName, m_displays[0].displayId, maxAveragePercentErrorPerPixel, subimageX, subimageY, subimageWidth, subimageHeight);
}

void RendererTestsFramework::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
{
    m_testRenderer.setFrameTimerLimits(limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
    m_testRenderer.flushRenderer();
}

bool RendererTestsFramework::compareScreenshotInternal(
    const ramses_internal::String& expectedImageName,
    ramses::displayId_t displayId,
    float maxAveragePercentErrorPerPixel,
    ramses_internal::UInt32 subimageX,
    ramses_internal::UInt32 subimageY,
    ramses_internal::UInt32 subimageWidth,
    ramses_internal::UInt32 subimageHeight)
{
    if (m_generateScreenshots)
    {
        m_testRenderer.saveScreenshotForDisplay(displayId, subimageX, subimageY, subimageWidth, subimageHeight, expectedImageName);
        return true;
    }

    bool comparisonResult = m_testRenderer.performScreenshotCheck(displayId, subimageX, subimageY, subimageWidth, subimageHeight, expectedImageName, maxAveragePercentErrorPerPixel);

    if (!comparisonResult)
    {
        assert(m_activeTestCase != NULL);
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Screenshot comparison failed for rendering test case: " << m_activeTestCase->m_name << " -> expected screenshot: " << expectedImageName);
    }

    return comparisonResult;
}

bool RendererTestsFramework::NameMatchesFilter(const ramses_internal::String& name, const ramses_internal::StringVector& filters)
{
    if (contains_c(filters, "*"))
    {
        return true;
    }

    for(const auto& filter : filters)
    {
        if (name.find(filter) >= 0)
        {
            return true;
        }
    }

    return false;
}

void RendererTestsFramework::filterTestCases(const ramses_internal::StringVector& filterIn, const ramses_internal::StringVector& filterOut)
{
    const bool processFilterIn = !filterIn.empty();
    const bool processFilterOut = !filterOut.empty();

    if (!processFilterIn && !processFilterOut)
    {
        return;
    }

    RenderingTestCases testCases;
    testCases.swap(m_testCases);

    for(const auto& testCase : testCases)
    {
        const ramses_internal::String testCaseName = testCase->m_name;
        const bool excludedByFilterIn = processFilterIn && !NameMatchesFilter(testCaseName, filterIn);
        const bool excludedByFilterOut = processFilterOut && NameMatchesFilter(testCaseName, filterOut);

        if (excludedByFilterIn || excludedByFilterOut)
        {
            delete testCase;
        }
        else
        {
            m_testCases.push_back(testCase);
        }
    }
}

bool areEqual(const DisplayConfigVector& a, const DisplayConfigVector& b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (ramses_internal::UInt32 i = 0u; i < a.size(); ++i)
    {
        const ramses_internal::DisplayConfig& displayConfigA = a[i].impl.getInternalDisplayConfig();
        const ramses_internal::DisplayConfig& displayConfigB = b[i].impl.getInternalDisplayConfig();

        if (displayConfigA != displayConfigB)
        {
            return false;
        }
    }

    return true;
}

void RendererTestsFramework::sortTestCases()
{
    // split test cases into groups using the same renderer/displays setup
    const ramses_internal::UInt numCases = m_testCases.size();

    RenderingTestCases unsortedCases;
    unsortedCases.swap(m_testCases);
    std::vector<bool> processedFlags(numCases, false);

    while (m_testCases.size() != numCases)
    {
        ramses_internal::UInt32 nextUnprocessed = 0u;
        while (processedFlags[nextUnprocessed])
        {
            ++nextUnprocessed;
        }
        m_testCases.push_back(unsortedCases[nextUnprocessed]);
        processedFlags[nextUnprocessed] = true;

        const DisplayConfigVector& groupSetup = unsortedCases[nextUnprocessed]->m_displayConfigs;
        for (ramses_internal::UInt32 i = nextUnprocessed + 1u; i < numCases; ++i)
        {
            if (processedFlags[i])
            {
                continue;
            }

            RenderingTestCase* testCase = unsortedCases[i];
            if (areEqual(groupSetup, testCase->m_displayConfigs))
            {
                m_testCases.push_back(testCase);
                processedFlags[i] = true;
            }
        }
    }
}

bool RendererTestsFramework::currentDisplaySetupMatchesTestCase(const RenderingTestCase& testCase) const
{
    if (testCase.m_displayConfigs.size() != m_displays.size())
    {
        return false;
    }

    for (ramses_internal::UInt i = 0u; i < m_displays.size(); ++i)
    {
        assert(i < m_displays.size());

        ramses_internal::DisplayConfig currentDisplayConfig = m_displays[i].config.impl.getInternalDisplayConfig();
        ramses_internal::DisplayConfig requestedDisplayConfig = testCase.m_displayConfigs[i].impl.getInternalDisplayConfig();

        // ignore wayland ID in comparison as this is different for every test display config
        requestedDisplayConfig.setWaylandIviSurfaceID(currentDisplayConfig.getWaylandIviSurfaceID());

        if (currentDisplayConfig != requestedDisplayConfig)
        {
            return false;
        }
    }

    return true;
}

bool RendererTestsFramework::applyRendererAndDisplaysConfigurationForTest(const RenderingTestCase& testCase)
{
    if(!testCase.m_defaultRendererRequired)
    {
        //destroy renderer and displays if needed
        if(m_testRenderer.isRendererInitialized())
        {
            destroyDisplays();
            destroyRenderer();
        }

        return true;
    }

    //create renderer and displays if needed
    if(!m_testRenderer.isRendererInitialized())
        initializeRenderer();

    if (currentDisplaySetupMatchesTestCase(testCase))
    {
        return true;
    }

    destroyDisplays();

    for(const auto& displayConfig : testCase.m_displayConfigs)
    {
        const ramses::displayId_t displayId = createDisplay(displayConfig);

        if (displayId == ramses::InvalidDisplayId)
        {
            return false;
        }

        if (displayConfig.impl.getInternalDisplayConfig().isWarpingEnabled())
        {
            // Use default test warping mesh for render tests using warped display
            m_testRenderer.updateWarpingMeshData(displayId, RendererTestUtils::CreateTestWarpingMesh());
        }
    }

    return true;
}

void RendererTestsFramework::destroyDisplays()
{
    for(const auto& display : m_displays)
    {
        m_testRenderer.destroyDisplay(display.displayId);
    }
    m_displays.clear();
}

void RendererTestsFramework::destroyScenes()
{
    m_testRenderer.getScenesRegistry().destroyScenes();
}

void RendererTestsFramework::destroyOffscreenBuffers()
{
    for (auto& display : m_displays)
    {
        OffscreenBufferVector& buffers = display.offscreenBuffers;

        for(const auto& buffer : buffers)
        {
            m_testRenderer.destroyOffscreenBuffer(display.displayId, buffer);
        }
        buffers.clear();
    }
}

bool RendererTestsFramework::runAllTests()
{
    bool testResult = true;

    const ramses_internal::UInt64 startTime = ramses_internal::PlatformTime::GetMillisecondsMonotonic();

    assert(m_activeTestCase == NULL);
    m_passedTestCases.clear();
    m_failedTestCases.clear();

    sortTestCases();

    for(const auto& testCase : m_testCases)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "Running rendering test case: " << testCase->m_name);
        printf("Running rendering test case: %s\n", testCase->m_name.c_str());
        fflush(stdout);

        if (applyRendererAndDisplaysConfigurationForTest(*testCase))
        {
            testResult &= runTestCase(*testCase);
        }
        else
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Renderer/display initialization failed for rendering test case: " << testCase->m_name);
            testResult = false;
        }

        destroyScenes();
        destroyOffscreenBuffers();
    }

    const ramses_internal::UInt64 endTime = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
    m_elapsedTime = endTime - startTime;

    return testResult;
}

bool RendererTestsFramework::runTestCase(RenderingTestCase& testCase)
{
    m_activeTestCase = &testCase;

    const bool testResult = m_activeTestCase->m_rendererTest.run(*this, testCase);
    if (testResult)
    {
        m_passedTestCases.push_back(m_activeTestCase);
    }
    else
    {
        m_failedTestCases.push_back(m_activeTestCase);
    }

    m_activeTestCase = NULL;

    return testResult;
}

ramses_internal::String RendererTestsFramework::generateReport() const
{
    ramses_internal::StringOutputStream str;

    str << "\n\n--- Rendering test report begin ---\n";
    {
        str << "\n  Passed rendering test cases: " << m_passedTestCases.size();
        for(const auto& testCase : m_passedTestCases)
        {
            str << "\n    " << testCase->m_name;
        }

        str << "\n\n  Failed rendering test cases: " << m_failedTestCases.size();
        for (const auto& testCase : m_failedTestCases)
        {
            str << "\n    " << testCase->m_name;
        }

        if (m_failedTestCases.empty())
        {
            str << "\n";
            str << "\n  ------------------";
            str << "\n  --- ALL PASSED ---";
            str << "\n  ------------------";
        }
        else
        {
            str << "\n";
            str << "\n  !!!!!!!!!!!!!!!!!!!!";
            str << "\n  !!! FAILED TESTS !!!";
            str << "\n  !!!!!!!!!!!!!!!!!!!!";
        }

        str.setDecimalDigits(2u);
        str << "\n\n  Total time elapsed: " << static_cast<float>(m_elapsedTime) * 0.001f << " s";
    }
    str << "\n\n--- End of rendering test report ---\n\n";

    return str.release();
}

const RendererTestsFramework::TestDisplays& RendererTestsFramework::getDisplays() const
{
    return m_displays;
}
