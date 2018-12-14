//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRESSTEST_H
#define RAMSES_STRESSTEST_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Collections/String.h"
#include <memory>

namespace ramses
{
    class RenderPass;
    class RenderGroup;
    class Scene;
    class RamsesRenderer;
    class RamsesClient;
    class RamsesFramework;
    class OrthographicCamera;
    class Node;
}
class MemoryLogger;

class StressTest
{
public:
    StressTest(int32_t argc, const char* argv[], const ramses_internal::String& name);
    virtual ~StressTest();

    // handle assignment and copy, as we have raw pointers
    // we have to delete copy and assignment
    StressTest(const StressTest&)            = delete;
    StressTest& operator=(const StressTest&) = delete;

    void init();

    int32_t run();
    uint64_t runningTimeMs() const;
    uint32_t runningLoops()  const;

    const ramses_internal::String& name() const;

protected:
    virtual void createSceneElements();
    virtual void destroyScene();

    virtual int32_t run_pre();
    virtual int32_t run_loop() = 0;


    void createAndShowScene();
    void showScene(ramses::sceneId_t sceneId);
    bool isScreenshotSimilar(const ramses_internal::String &screenshotName, float maxPercentErrorPerPixel) const;
    bool isCycleBased() const;
    bool isStillLooping() const;
    bool isMemLoggingNeeded(const MemoryLogger &logger) const;


    const ramses_internal::String m_name;
    const uint32_t                m_durationEachTestSeconds;
    const uint32_t                m_durationEachTestCycles;
    const uint32_t                m_numMemorySamples;
    const uint32_t                m_displayWidth         = 300;
    const uint32_t                m_displayHeight        = 100;
    ramses::RamsesFramework*      m_framework            = nullptr;
    ramses::RamsesRenderer*       m_renderer             = nullptr;
    ramses::RamsesClient*         m_client               = nullptr;
    ramses::Scene*                m_clientScene          = nullptr;
    ramses::RenderPass*           m_renderPass           = nullptr;
    ramses::RenderGroup*          m_renderGroup          = nullptr;
    ramses::Node*                 m_cameraTranslation    = nullptr;
    ramses::OrthographicCamera*   m_camera               = nullptr;
    ramses::displayId_t           m_displayId            = ramses::InvalidDisplayId;
    ramses_internal::Bool         m_firstRun             = true;

private:
    void startTiming();
    void tick();

    uint64_t                      m_startTimeMs          = 0u;
    uint64_t                      m_currentTimeMs        = 0u;
    uint64_t                      m_logPeriod            = 0u;
    uint32_t                      m_loopCounter          = 0u;
};

using StressTestPtr = std::unique_ptr< StressTest >;

#endif
