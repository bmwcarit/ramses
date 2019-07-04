//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "DisplayManager/DisplayManager.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "Scene/ClientScene.h"

using namespace ramses_display_manager;
using namespace testing;

class SceneRendererStateTracker final : public ramses::RendererEventHandlerEmpty
{
public:
    explicit SceneRendererStateTracker(ramses::sceneId_t sceneId)
        : m_sceneId(sceneId)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
        {
            m_lastState = SceneState::Unavailable;
            if (m_wasUnpublished)
                m_wasRepublished = true;
        }
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Available;
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Ready;
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Rendered;
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
        {
            m_lastState = SceneState::Unavailable;
            m_wasUnpublished = true;
        }
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Unavailable;
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Available;
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = SceneState::Ready;
    }

    SceneState getLastState() const
    {
        return m_lastState;
    }

    bool wasUnpublishedAndRepublished() const
    {
        return m_wasUnpublished && m_wasRepublished;
    }

private:
    const ramses::sceneId_t m_sceneId;
    SceneState m_lastState = SceneState::Unavailable;
    bool m_wasUnpublished = false;
    bool m_wasRepublished = false;
};

class SceneStateTracker final : public IEventHandler
{
public:
    explicit SceneStateTracker(ramses::sceneId_t sceneId)
        : m_sceneId(sceneId)
    {
    }

    virtual void sceneStateChanged(ramses::sceneId_t sceneId, SceneState state, ramses::displayId_t displaySceneIsMappedTo) override
    {
        if (sceneId == m_sceneId)
        {
            m_lastState = state;
            m_displayMappedTo = displaySceneIsMappedTo;
        }
    }

    const ramses::sceneId_t m_sceneId;
    SceneState m_lastState = SceneState::Unavailable;
    ramses::displayId_t m_displayMappedTo = ramses::InvalidDisplayId;
};

class RamsesClientAndRendererWithScene
{
public:
    RamsesClientAndRendererWithScene()
        : m_renderer(m_framework, {})
        , m_client("client", m_framework)
        , m_displayManager(m_renderer, m_framework, false)
        , m_sceneId(33u)
        , m_scene(*m_client.createScene(m_sceneId))
    {
        ramses::DisplayConfig displayConfig;
        displayConfig.setWaylandIviSurfaceID(0u);
        m_displayId = m_displayManager.createDisplay(displayConfig);
        m_displayManager.dispatchAndFlush();
        m_renderer.doOneLoop();
        m_displayManager.dispatchAndFlush();

        // flush at least once so that scene can be subscribed
        m_scene.flush();

        // set default mapping
        m_displayManager.setSceneMapping(m_sceneId, m_displayId);
    }

    ramses::RamsesFramework m_framework;
    ramses::RamsesRenderer m_renderer;
    ramses::RamsesClient m_client;
    DisplayManager m_displayManager;

    ramses::displayId_t m_displayId;
    ramses::sceneId_t m_sceneId;
    ramses::Scene& m_scene;
};

class ADisplayManagerWithRenderer : public Test
{
public:
    ADisplayManagerWithRenderer()
        : m_displayManagerEventHandler(ramsesClientRenderer->m_displayManager)
        , m_sceneStateTracker(ramsesClientRenderer->m_sceneId)
        , m_sceneRendererStateTracker(ramsesClientRenderer->m_sceneId)
    {
    }
    virtual void SetUp() override
    {
        ramsesClientRenderer->m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
    }

    virtual void TearDown() override
    {
        ramsesClientRenderer->m_scene.unpublish();
        ramsesClientRenderer->m_renderer.doOneLoop();
        dispatchAndFlush();
    }

    static void SetUpTestCase()
    {
        ramsesClientRenderer = new RamsesClientAndRendererWithScene;
    }

    static void TearDownTestCase()
    {
        delete ramsesClientRenderer;
        ramsesClientRenderer = nullptr;
    }

protected:
    void dispatchAndFlush()
    {
        expectSyncedSceneStates();
        ramsesClientRenderer->m_displayManager.dispatchAndFlush(&m_sceneStateTracker, &m_sceneRendererStateTracker);
        expectSyncedSceneStates();
    }

    void expectSyncedSceneStates()
    {
        const auto lastDMState = ramsesClientRenderer->m_displayManager.getLastReportedSceneState(ramsesClientRenderer->m_sceneId);
        const auto lastTrackedState = m_sceneStateTracker.m_lastState;
        const auto lastTrackedRendererState = m_sceneRendererStateTracker.getLastState();
        EXPECT_EQ(lastTrackedState, lastDMState);
        EXPECT_EQ(lastTrackedRendererState, lastDMState);
        const auto expectedDisplay = (lastTrackedState == SceneState::Ready || lastTrackedState == SceneState::Rendered) ? ramsesClientRenderer->m_displayId : ramses::InvalidDisplayId;
        EXPECT_EQ(expectedDisplay, m_sceneStateTracker.m_displayMappedTo);
    }

    ramses::IRendererEventHandler& m_displayManagerEventHandler;

    SceneStateTracker m_sceneStateTracker;
    SceneRendererStateTracker m_sceneRendererStateTracker;

    static RamsesClientAndRendererWithScene* ramsesClientRenderer;

    static constexpr int NumStates = GetNumSceneStates();
    static constexpr int NumLoopsToReachRenderedTargetState = 6; // from published to rendered
};

RamsesClientAndRendererWithScene* ADisplayManagerWithRenderer::ramsesClientRenderer = nullptr;

TEST_F(ADisplayManagerWithRenderer, willShowSceneWhenPublished)
{
    ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, SceneState::Rendered);

    for (int i = 0; i < NumLoopsToReachRenderedTargetState; ++i)
    {
        ramsesClientRenderer->m_renderer.doOneLoop();
        dispatchAndFlush();
    }

    EXPECT_EQ(SceneState::Rendered, ramsesClientRenderer->m_displayManager.getLastReportedSceneState(ramsesClientRenderer->m_sceneId));
    EXPECT_EQ(SceneState::Rendered, m_sceneStateTracker.m_lastState);
    EXPECT_EQ(SceneState::Rendered, m_sceneRendererStateTracker.getLastState());
}

/////////
// Combinatorial tests where every combination of: target state is set and new target state is set at some point
/////////
class ADisplayManagerWithRenderer_TargetToTarget : public ADisplayManagerWithRenderer, public WithParamInterface<int>
{
public:
    static constexpr int NumLoopsToReachTargetState = NumLoopsToReachRenderedTargetState;
    static constexpr int NumStepsToReachTargetState = NumLoopsToReachTargetState * 2; // 2 steps per loop
    static constexpr int NumCombinations = NumStepsToReachTargetState * NumStates * NumStates; // can switch to new state at any step

    SceneState GetTargetState1() const
    {
        return SceneState((GetParam() / NumStepsToReachTargetState) / NumStates);
    }

    SceneState GetTargetState2() const
    {
        return SceneState((GetParam() / NumStepsToReachTargetState) % NumStates);
    }

    int GetStepToSwitchState() const
    {
        return GetParam() % NumStepsToReachTargetState;
    }
};

INSTANTIATE_TEST_CASE_P(TargetToTarget, ADisplayManagerWithRenderer_TargetToTarget, Range(0, ADisplayManagerWithRenderer_TargetToTarget::NumCombinations));

TEST_P(ADisplayManagerWithRenderer_TargetToTarget, switchFromTargetStateToTargetStateAtEveryStep)
{
    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "Testing switch from target state " << SceneStateName(GetTargetState1()) << " to target state " << SceneStateName(GetTargetState2()) << " at step " << GetStepToSwitchState());

    ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState1());

    int step = 0;
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToSwitchState())
            ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState2());
        ramsesClientRenderer->m_renderer.doOneLoop();
        ++step;

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToSwitchState())
            ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState2());
        dispatchAndFlush();
        ++step;
    }

    // execute few more loops to allow DM to get to target state
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        ramsesClientRenderer->m_renderer.doOneLoop();
        dispatchAndFlush();
    }

    // expect target state reached and synced
    EXPECT_EQ(GetTargetState2(), ramsesClientRenderer->m_displayManager.getLastReportedSceneState(ramsesClientRenderer->m_sceneId));
    expectSyncedSceneStates();
}


/////////
// Combinatorial tests where unpublish and republish come in every possible time during getting scene to all possible states
/////////
class ADisplayManagerWithRenderer_ReachTargetStateWithUnpublish : public ADisplayManagerWithRenderer, public WithParamInterface<int>
{
public:
    static constexpr int MaxNumStepsRepublishComesAfter = 4;
    static constexpr int NumLoopsToReachTargetState = NumLoopsToReachRenderedTargetState + MaxNumStepsRepublishComesAfter; // add room for re/publish combinations
    static constexpr int NumStepsToReachTargetState = NumLoopsToReachTargetState * 2; // 2 steps per loop
    static constexpr int NumCombinations = (NumStepsToReachTargetState - 5) * MaxNumStepsRepublishComesAfter * NumStates; // can unpublish at any step, with room to process (-5), and encodes when publish comes (*MaxNumStepsRepublishComesAfter) and target state

    SceneState GetTargetState() const
    {
        return SceneState(GetParam() % NumStates);
    }

    int GetStepToUnpublish() const
    {
        return (GetParam() / NumStates) / MaxNumStepsRepublishComesAfter;
    }

    int GetStepToRepublish() const
    {
        // either at same step or one step
        return GetStepToUnpublish() + (GetParam() / NumStates) % MaxNumStepsRepublishComesAfter;
    }
};

INSTANTIATE_TEST_CASE_P(ReachTargetStateWithUnpublish, ADisplayManagerWithRenderer_ReachTargetStateWithUnpublish, Range(0, ADisplayManagerWithRenderer_ReachTargetStateWithUnpublish::NumCombinations));

TEST_P(ADisplayManagerWithRenderer_ReachTargetStateWithUnpublish, unpublishAtEveryStepWhileReachingEveryState)
{
    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "Testing unpublish from target state " << SceneStateName(GetTargetState()) << " at step " << GetStepToUnpublish() << " and re-publish at step " << GetStepToRepublish());

    ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState());

    int step = 0;
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToUnpublish())
            ramsesClientRenderer->m_scene.unpublish();
        if (step == GetStepToRepublish())
            ramsesClientRenderer->m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
        ramsesClientRenderer->m_renderer.doOneLoop();
        ++step;

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToUnpublish())
            ramsesClientRenderer->m_scene.unpublish();
        if (step == GetStepToRepublish())
            ramsesClientRenderer->m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
        dispatchAndFlush();
        ++step;
    }
    // at this point scene was unpublished and republished
    ASSERT_TRUE(m_sceneRendererStateTracker.wasUnpublishedAndRepublished());

    // execute few more loops to allow DM to get to last state
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        ramsesClientRenderer->m_renderer.doOneLoop();
        dispatchAndFlush();
    }
    EXPECT_EQ(GetTargetState(), ramsesClientRenderer->m_displayManager.getLastReportedSceneState(ramsesClientRenderer->m_sceneId));
    expectSyncedSceneStates();

    // test that rendered state can be reached, regardless of previous target state
    ramsesClientRenderer->m_displayManager.setSceneState(ramsesClientRenderer->m_sceneId, SceneState::Rendered);

    // execute few more loops to allow DM to get to rendered state
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        ramsesClientRenderer->m_renderer.doOneLoop();
        dispatchAndFlush();
    }

    // expect shown and synced
    EXPECT_EQ(SceneState::Rendered, ramsesClientRenderer->m_displayManager.getLastReportedSceneState(ramsesClientRenderer->m_sceneId));
    expectSyncedSceneStates();
}
