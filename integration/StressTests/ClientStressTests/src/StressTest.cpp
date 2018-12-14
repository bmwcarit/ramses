//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StressTest.h"

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/OrthographicCamera.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "Utils/Argument.h"
#include "RendererTestEventHandler.h"
#include "RendererTestUtils.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "MemoryLogger.h"

using namespace ramses_internal;

StressTest::StressTest(int32_t argc, const char* argv[], const ramses_internal::String& name)
    : m_name(name)
    , m_durationEachTestSeconds(ArgumentUInt32(CommandLineParser(argc, argv), "dur", "duration-each-test-seconds", 1))
    , m_durationEachTestCycles(ArgumentUInt32(CommandLineParser(argc,argv), "cyc", "duration-test-cycles", 0))
    , m_numMemorySamples(ArgumentUInt32(CommandLineParser(argc, argv), "ms", "memory-samples", 0))
{
    ramses::RendererConfig rendererConfig(argc, argv);
    m_framework = new ramses::RamsesFramework(argc, argv);
    m_client    = new ramses::RamsesClient("ramses-stress-test-client", *m_framework);
    m_renderer  = new ramses::RamsesRenderer(*m_framework, rendererConfig);

    ramses::DisplayConfig displayConfig(argc, argv);
    displayConfig.setWindowRectangle(50, 50, m_displayWidth, m_displayHeight);

    if (0xFFFFFFFF == displayConfig.getIntegrityEGLDisplayID())
    {
        displayConfig.setIntegrityEGLDisplayID(100);
    }

    if(0xFFFFFFFF == displayConfig.getWaylandIviSurfaceID())
    {
        displayConfig.setWaylandIviSurfaceID(0);
    }

    if(0xFFFFFFFF == displayConfig.getWaylandIviLayerID())
    {
        displayConfig.setWaylandIviLayerID(3);
    }

    m_displayId = m_renderer->createDisplay(displayConfig);

    m_framework->connect();

    if (m_numMemorySamples > 0)
    {
        if (isCycleBased())
        {
            m_logPeriod = m_durationEachTestCycles / m_numMemorySamples;
        }
        else
        {
            m_logPeriod = m_durationEachTestSeconds * 1000 / m_numMemorySamples;
        }
    }
}

StressTest::~StressTest()
{
    destroyScene();

    delete m_renderer;
    delete m_client;
    delete m_framework;
}

void StressTest::destroyScene()
{
    if(m_client && m_clientScene)
    {
        m_client->destroy(*m_clientScene);
        m_clientScene       = nullptr;
        m_camera            = nullptr;
        m_cameraTranslation = nullptr;
        m_renderGroup       = nullptr;
        m_renderPass        = nullptr;
    }
}

void StressTest::init()
{
    createAndShowScene();
}

void StressTest::createSceneElements()
{
    assert(m_client);
    assert(m_clientScene);
}

void StressTest::createAndShowScene()
{
    destroyScene();

    //client scene
    const ramses::sceneId_t sceneId = 1u;
    m_clientScene = m_client->createScene(sceneId);

    //camera
    m_cameraTranslation = m_clientScene->createNode("camera translation");
    m_cameraTranslation->setTranslation(0.0f, 0.0f, 0.5f);
    m_camera = m_clientScene->createOrthographicCamera("camera");
    m_camera->setFrustum(0.0f, static_cast<Float>(m_displayWidth), 0.0f, static_cast<Float>(m_displayHeight), 0.1f, 1.f);
    m_camera->setViewport(0, 0, m_displayWidth, m_displayHeight);
    m_camera->setParent(*m_cameraTranslation);

    m_renderPass = m_clientScene->createRenderPass("render pass");
    m_renderPass->setClearFlags(ramses::EClearFlags_None);
    m_renderPass->setCamera(*m_camera);

    m_renderGroup = m_clientScene->createRenderGroup("render group");
    m_renderPass->addRenderGroup(*m_renderGroup);

    createSceneElements();

    m_clientScene->flush();
    m_clientScene->publish(ramses::EScenePublicationMode_LocalOnly);

    showScene(sceneId);
}

void StressTest::showScene(ramses::sceneId_t sceneId)
{
    RendererTestEventHandler stateEventHandler(*m_renderer);

    stateEventHandler.waitForPublication(sceneId);

    m_renderer->subscribeScene(sceneId);
    m_renderer->flush();
    stateEventHandler.waitForSubscription(sceneId);

    m_renderer->mapScene(m_displayId, sceneId);
    m_renderer->flush();
    stateEventHandler.waitForMapped(sceneId);

    m_renderer->showScene(sceneId);
    m_renderer->flush();
    stateEventHandler.waitForShown(sceneId);
}

const String& StressTest::name() const
{
    return m_name;
}


bool StressTest::isScreenshotSimilar(const String& screenshotName, float maxPercentErrorPerPixel) const
{
    return RendererTestUtils::PerformScreenshotTestForDisplay(*m_renderer, m_displayId, 0u, 0u, m_displayWidth, m_displayHeight, screenshotName, maxPercentErrorPerPixel);
}

int32_t StressTest::run_pre()
{
    return 0;
}

int32_t StressTest::run()
{
    int32_t returnValue = run_pre();
    MemoryLogger memlogger(m_numMemorySamples);

    m_firstRun = true;
    for (startTiming(); returnValue == 0 && isStillLooping(); tick())
    {
        if (isMemLoggingNeeded(memlogger))
        {
            memlogger.logMemory();
        }

        returnValue = run_loop();

        m_firstRun = false;
    }
    memlogger.writeLogToFile(String("StressTestResults/ClientStressTest-") + name());

    return returnValue;
}

uint64_t StressTest::runningTimeMs() const
{
    return m_currentTimeMs - m_startTimeMs;
}

uint32_t StressTest::runningLoops() const
{
    return m_loopCounter;
}

bool StressTest::isCycleBased() const
{
    return m_durationEachTestCycles > 0;
}

bool StressTest::isStillLooping() const
{
    if( isCycleBased() )
    {
        return m_loopCounter < m_durationEachTestCycles;
    }
    else
    {
        return runningTimeMs() / 1000 < m_durationEachTestSeconds;
    }
}

bool StressTest::isMemLoggingNeeded(const MemoryLogger &logger) const
{
    if (m_numMemorySamples == 0)
    {
        return false;
    }

    const uint32_t timesLogged = static_cast<uint32_t>(logger.getTimesMemoryLogged());
    const uint64_t loggedPeriod = static_cast<uint64_t>(timesLogged) * m_logPeriod;

    if (isCycleBased())
    {
        return m_loopCounter >= loggedPeriod;
    }
    else
    {
        return runningTimeMs() >= loggedPeriod;
    }
}

void StressTest::startTiming()
{
    m_startTimeMs   = PlatformTime::GetMillisecondsMonotonic();
    m_currentTimeMs = m_startTimeMs;
    m_loopCounter   = 0u;
}

void StressTest::tick()
{
    m_currentTimeMs = PlatformTime::GetMillisecondsMonotonic();
    ++m_loopCounter;
}
