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
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "RendererTestUtils.h"

RendererTestsFramework::RendererTestsFramework(bool generateScreenshots, const ramses::RamsesFrameworkConfig& config)
    : m_generateScreenshots(generateScreenshots)
    , m_testScenesAndRenderer(config)
    , m_testRenderer(m_testScenesAndRenderer.getTestRenderer())
    , m_activeTestCase(nullptr)
    , m_elapsedTime(0u)
{
}

RendererTestsFramework::~RendererTestsFramework()
{
    assert(m_activeTestCase == nullptr);
    for(const auto& testCase : m_testCases)
    {
        delete testCase;
    }

    destroyDisplays();
    m_testScenesAndRenderer.destroyRenderer();
}

void RendererTestsFramework::initializeRenderer()
{
    m_testScenesAndRenderer.initializeRenderer();
}

void RendererTestsFramework::initializeRenderer(const ramses::RendererConfig& rendererConfig)
{
    m_testScenesAndRenderer.initializeRenderer(rendererConfig);
}

void RendererTestsFramework::destroyRenderer()
{
    m_testScenesAndRenderer.destroyRenderer();
}

ramses::displayId_t RendererTestsFramework::createDisplay(const ramses::DisplayConfig& displayConfig)
{
    m_testRenderer.setLoopMode(ramses::ELoopMode::UpdateOnly);
    const ramses::displayId_t displayId = m_testRenderer.createDisplay(displayConfig);
    m_testRenderer.setLoopMode(ramses::ELoopMode::UpdateAndRender);
    if (displayId != ramses::displayId_t::Invalid())
    {
        m_displays.push_back({ displayId, displayConfig, {}, {} });
    }

    return displayId;
}

ramses::displayBufferId_t RendererTestsFramework::getDisplayFramebufferId(uint32_t testDisplayIdx) const
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    return m_testRenderer.getDisplayFramebufferId(displayId);
}

ramses_internal::TestRenderer& RendererTestsFramework::getTestRenderer()
{
    return m_testRenderer;
}

RenderingTestCase& RendererTestsFramework::createTestCase(uint32_t id, IRendererTest& rendererTest, const std::string& name)
{
    RenderingTestCase* testCase = new RenderingTestCase(id, rendererTest, name, true);
    m_testCases.push_back(testCase);

    return *testCase;
}

RenderingTestCase& RendererTestsFramework::createTestCaseWithDefaultDisplay(uint32_t id, IRendererTest& rendererTest, const std::string& name, bool iviWindowStartVisible)
{
    RenderingTestCase& testCase = createTestCase(id, rendererTest, name);
    ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(0, iviWindowStartVisible);
    displayConfig.setWindowRectangle(0, 0, ramses_internal::IntegrationScene::DefaultViewportWidth, ramses_internal::IntegrationScene::DefaultViewportHeight);
    testCase.m_displayConfigs.push_back(displayConfig);

    return testCase;
}

RenderingTestCase& RendererTestsFramework::createTestCaseWithoutRenderer(uint32_t id, IRendererTest &rendererTest, const std::string &name)
{
    RenderingTestCase* testCase = new RenderingTestCase(id, rendererTest, name, false);
    m_testCases.push_back(testCase);

    return *testCase;
}

TestScenes& RendererTestsFramework::getScenesRegistry()
{
    return m_testScenesAndRenderer.getScenesRegistry();
}

ramses::RamsesClient& RendererTestsFramework::getClient()
{
    return m_testScenesAndRenderer.getClient();
}

void RendererTestsFramework::setSceneMapping(ramses::sceneId_t sceneId, uint32_t testDisplayIdx)
{
    m_testRenderer.setSceneMapping(sceneId, m_displays[testDisplayIdx].displayId);
}

bool RendererTestsFramework::getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
{
    return m_testRenderer.getSceneToState(sceneId, state);
}

bool RendererTestsFramework::getSceneToRendered(ramses::sceneId_t sceneId, uint32_t testDisplayIdx)
{
    setSceneMapping(sceneId, testDisplayIdx);
    return getSceneToState(sceneId, ramses::RendererSceneState::Rendered);
}

void RendererTestsFramework::dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler, ramses::IRendererSceneControlEventHandler& sceneControlEventHandler)
{
    m_testRenderer.dispatchEvents(eventHandler, sceneControlEventHandler);
}

ramses::displayBufferId_t RendererTestsFramework::createOffscreenBuffer(uint32_t testDisplayIdx, uint32_t width, uint32_t height, bool interruptible, uint32_t sampleCount, ramses::EDepthBufferType depthBufferType)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    ramses::displayBufferId_t buffer = m_testRenderer.createOffscreenBuffer(displayId, width, height, interruptible, sampleCount, depthBufferType);
    m_displays[testDisplayIdx].offscreenBuffers.push_back(buffer);
    return buffer;
}

ramses::displayBufferId_t RendererTestsFramework::createDmaOffscreenBuffer(uint32_t testDisplayIdx, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t bufferUsageFlags, uint64_t modifier)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    ramses::displayBufferId_t buffer = m_testRenderer.createDmaOffscreenBuffer(displayId, width, height, bufferFourccFormat, bufferUsageFlags, modifier);
    m_displays[testDisplayIdx].offscreenBuffers.push_back(buffer);
    return buffer;
}

bool RendererTestsFramework::getDmaOffscreenBufferFDAndStride(uint32_t testDisplayIdx, ramses::displayBufferId_t displayBufferId, int &fd, uint32_t &stride) const
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    return m_testRenderer.getDmaOffscreenBufferFDAndStride(displayId, displayBufferId, fd, stride);
}

void RendererTestsFramework::destroyOffscreenBuffer(uint32_t testDisplayIdx, ramses::displayBufferId_t buffer)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    auto& offscreenBuffers = m_displays[testDisplayIdx].offscreenBuffers;

    auto bufferIter = ramses_internal::find_c(offscreenBuffers, buffer);
    assert(bufferIter != offscreenBuffers.end());
    offscreenBuffers.erase(bufferIter);

    m_testRenderer.destroyOffscreenBuffer(displayId, buffer);
}

ramses::streamBufferId_t RendererTestsFramework::createStreamBuffer(uint32_t testDisplayIdx, ramses::waylandIviSurfaceId_t source)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    ramses::streamBufferId_t buffer = m_testRenderer.createStreamBuffer(displayId, source);
    m_displays[testDisplayIdx].streamBuffers.push_back(buffer);
    return buffer;
}

void RendererTestsFramework::destroyStreamBuffer(uint32_t testDisplayIdx, ramses::streamBufferId_t buffer)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    auto& streamBuffers = m_displays[testDisplayIdx].streamBuffers;

    auto bufferIter = ramses_internal::find_c(streamBuffers, buffer);
    assert(bufferIter != streamBuffers.end());
    streamBuffers.erase(bufferIter);

    m_testRenderer.destroyStreamBuffer(displayId, buffer);
}

void RendererTestsFramework::assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder)
{
    m_testRenderer.assignSceneToDisplayBuffer(sceneId, buffer, renderOrder);
}

void RendererTestsFramework::createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    m_testRenderer.createBufferDataLink(providerBuffer, consumerScene, consumerTag);
}

void RendererTestsFramework::createBufferDataLink(ramses::streamBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    m_testRenderer.createBufferDataLink(providerBuffer, consumerScene, consumerTag);
}

void RendererTestsFramework::createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerTag, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    m_testRenderer.createDataLink(providerScene, providerTag, consumerScene, consumerTag);
}

void RendererTestsFramework::removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    m_testRenderer.removeDataLink(consumerScene, consumerTag);
}

void RendererTestsFramework::setClearFlags(uint32_t testDisplayIdx, ramses::displayBufferId_t ob, uint32_t clearFlags)
{
    assert(testDisplayIdx < m_displays.size());
    m_testRenderer.setClearFlags(m_displays[testDisplayIdx].displayId, ob, clearFlags);
    // clearing state is persistent if display kept for next test, force re-init
    m_forceDisplaysReinitForNextTestCase = true;
}

void RendererTestsFramework::setClearColor(uint32_t testDisplayIdx, ramses::displayBufferId_t ob, const glm::vec4& clearColor)
{
    assert(testDisplayIdx < m_displays.size());
    m_testRenderer.setClearColor(m_displays[testDisplayIdx].displayId, ob, clearColor);
    // clear color change is persistent if display kept for next test, force re-init
    m_forceDisplaysReinitForNextTestCase = true;
}

void RendererTestsFramework::publishAndFlushScene(ramses::sceneId_t sceneId)
{
    m_testScenesAndRenderer.flush(sceneId);
    m_testScenesAndRenderer.publish(sceneId);
}

void RendererTestsFramework::flushRendererAndDoOneLoop()
{
    m_testRenderer.flushRenderer();
    m_testRenderer.doOneLoop();
}

bool RendererTestsFramework::renderAndCompareScreenshot(const std::string& expectedImageName, uint32_t testDisplayIdx, float maxAveragePercentErrorPerPixel, bool readPixelsTwice, bool saveDiffOnError)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;
    const ramses::DisplayConfig& displayConfig = m_displays[testDisplayIdx].config;

    return compareScreenshotInternal(
        expectedImageName,
        displayId,
        {},
        maxAveragePercentErrorPerPixel,
        0u,
        0u,
        displayConfig.m_impl.get().getInternalDisplayConfig().getDesiredWindowWidth(),
        displayConfig.m_impl.get().getInternalDisplayConfig().getDesiredWindowHeight(),
        readPixelsTwice,
        saveDiffOnError);
}

bool RendererTestsFramework::renderAndCompareScreenshotOffscreenBuffer(const std::string& expectedImageName, uint32_t testDisplayIdx, ramses::displayBufferId_t displayBuffer, uint32_t width, uint32_t height, float maxAveragePercentErrorPerPixel)
{
    assert(testDisplayIdx < m_displays.size());
    const ramses::displayId_t displayId = m_displays[testDisplayIdx].displayId;

    return compareScreenshotInternal(
        expectedImageName,
        displayId,
        displayBuffer,
        maxAveragePercentErrorPerPixel,
        0u,
        0u,
        width,
        height,
        false,
        true);
}

bool RendererTestsFramework::renderAndCompareScreenshotSubimage(const std::string& expectedImageName, uint32_t subimageX, uint32_t subimageY, uint32_t subimageWidth, uint32_t subimageHeight, float maxAveragePercentErrorPerPixel, bool readPixelsTwice)
{
    return compareScreenshotInternal(expectedImageName, m_displays[0].displayId, {}, maxAveragePercentErrorPerPixel, subimageX, subimageY, subimageWidth, subimageHeight, readPixelsTwice, true);
}

void RendererTestsFramework::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
{
    m_testRenderer.setFrameTimerLimits(limitForClientResourcesUpload, limitForOffscreenBufferRender);
    m_testRenderer.flushRenderer();
}

bool RendererTestsFramework::compareScreenshotInternal(
    const std::string& expectedImageName,
    ramses::displayId_t displayId,
    ramses::displayBufferId_t bufferId,
    float maxAveragePercentErrorPerPixel,
    uint32_t subimageX,
    uint32_t subimageY,
    uint32_t subimageWidth,
    uint32_t subimageHeight,
    bool readPixelsTwice,
    bool saveDiffOnError)
{
    if (m_generateScreenshots)
    {
        m_testRenderer.saveScreenshotForDisplay(displayId, bufferId, subimageX, subimageY, subimageWidth, subimageHeight, expectedImageName);
        return true;
    }

    const bool comparisonResult = m_testRenderer.performScreenshotCheck(displayId, bufferId, subimageX, subimageY, subimageWidth, subimageHeight, expectedImageName, maxAveragePercentErrorPerPixel, readPixelsTwice, saveDiffOnError);

    if (!comparisonResult && saveDiffOnError)
    {
        assert(m_activeTestCase != nullptr);
        LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "Screenshot comparison failed for rendering test case: " << m_activeTestCase->m_name << " -> expected screenshot: " << expectedImageName);
    }

    return comparisonResult;
}

bool RendererTestsFramework::NameMatchesFilter(const std::string& name, const std::vector<std::string>& filters)
{
    if (ramses_internal::contains_c(filters, "*"))
    {
        return true;
    }

    for(const auto& filter : filters)
    {
        if (name.find(filter) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

void RendererTestsFramework::filterTestCases(const std::vector<std::string>& filterIn, const std::vector<std::string>& filterOut)
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
        const auto& testCaseName = testCase->m_name;
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

    for (size_t i = 0; i < a.size(); ++i)
    {
        const ramses_internal::DisplayConfig& displayConfigA = a[i].m_impl.get().getInternalDisplayConfig();
        const ramses_internal::DisplayConfig& displayConfigB = b[i].m_impl.get().getInternalDisplayConfig();

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
    const size_t numCases = m_testCases.size();

    RenderingTestCases unsortedCases;
    unsortedCases.swap(m_testCases);
    std::vector<bool> processedFlags(numCases, false);

    while (m_testCases.size() != numCases)
    {
        uint32_t nextUnprocessed = 0u;
        while (processedFlags[nextUnprocessed])
        {
            ++nextUnprocessed;
        }
        m_testCases.push_back(unsortedCases[nextUnprocessed]);
        processedFlags[nextUnprocessed] = true;

        const DisplayConfigVector& groupSetup = unsortedCases[nextUnprocessed]->m_displayConfigs;
        for (uint32_t i = nextUnprocessed + 1u; i < numCases; ++i)
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

    for (size_t i = 0u; i < m_displays.size(); ++i)
    {
        assert(i < m_displays.size());

        ramses_internal::DisplayConfig currentDisplayConfig = m_displays[i].config.m_impl.get().getInternalDisplayConfig();
        ramses_internal::DisplayConfig requestedDisplayConfig = testCase.m_displayConfigs[i].m_impl.get().getInternalDisplayConfig();

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

    if (!m_forceDisplaysReinitForNextTestCase && currentDisplaySetupMatchesTestCase(testCase))
    {
        return true;
    }

    destroyDisplays();

    for(const auto& displayConfig : testCase.m_displayConfigs)
    {
        const ramses::displayId_t displayId = createDisplay(displayConfig);

        if (displayId == ramses::displayId_t::Invalid())
            return false;
    }
    m_forceDisplaysReinitForNextTestCase = false;

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
    m_testScenesAndRenderer.getScenesRegistry().destroyScenes();
}

void RendererTestsFramework::destroyBuffers()
{
    for (auto& display : m_displays)
    {
        for(const auto buffer : display.offscreenBuffers)
            m_testRenderer.destroyOffscreenBuffer(display.displayId, buffer);
        display.offscreenBuffers.clear();
        for (const auto buffer : display.streamBuffers)
            m_testRenderer.destroyStreamBuffer(display.displayId, buffer);
        display.streamBuffers.clear();
    }
}

bool RendererTestsFramework::runAllTests()
{
    bool testResult = true;

    const uint64_t startTime = ramses_internal::PlatformTime::GetMillisecondsMonotonic();

    assert(m_activeTestCase == nullptr);
    m_passedTestCases.clear();
    m_failedTestCases.clear();

    sortTestCases();

    for(const auto& testCase : m_testCases)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "====== Running rendering test case: " << testCase->m_name << " ======");
        printf("======\nRunning rendering test case: %s\n======\n", testCase->m_name.c_str());
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
        destroyBuffers();
    }

    const uint64_t endTime = ramses_internal::PlatformTime::GetMillisecondsMonotonic();
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

    m_activeTestCase = nullptr;

    return testResult;
}

std::string RendererTestsFramework::generateReport() const
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

        str << "\n\n  Total time elapsed: " << static_cast<float>(m_elapsedTime) * 0.001f << " s";
    }
    str << "\n\n--- End of rendering test report ---\n\n";

    return str.release();
}

const RendererTestsFramework::TestDisplays& RendererTestsFramework::getDisplays() const
{
    return m_displays;
}
