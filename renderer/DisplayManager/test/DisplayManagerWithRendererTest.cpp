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

class SceneStateTracker final : public ramses::RendererEventHandlerEmpty
{
public:
    explicit SceneStateTracker(ramses::sceneId_t sceneId)
        : m_sceneId(sceneId)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
        {
            m_lastState = DisplayManager::ESceneState::Published;
            if (m_wasUnpublished)
                m_wasRepublished = true;
        }
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Subscribed;
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Mapped;
    }

    virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Rendered;
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        if (sceneId == m_sceneId)
        {
            m_lastState = DisplayManager::ESceneState::Unpublished;
            m_wasUnpublished = true;
        }
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Published;
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Subscribed;
    }

    virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (sceneId == m_sceneId && result != ramses::ERendererEventResult_FAIL)
            m_lastState = DisplayManager::ESceneState::Mapped;
    }

    DisplayManager::ESceneState getLastState() const
    {
        return m_lastState;
    }

    bool wasUnpublishedAndRepublished() const
    {
        return m_wasUnpublished && m_wasRepublished;
    }

private:
    const ramses::sceneId_t m_sceneId;
    DisplayManager::ESceneState m_lastState = DisplayManager::ESceneState::Unpublished;
    bool m_wasUnpublished = false;
    bool m_wasRepublished = false;
};

class ADisplayManagerWithRenderer : public Test
{
public:
    ADisplayManagerWithRenderer()
        : m_renderer(m_framework, {})
        , m_client("client", m_framework)
        , m_displayManager(m_renderer, m_framework, false)
        , m_displayManagerEventHandler(m_displayManager)
        , m_sceneId(33u)
        , m_scene(*m_client.createScene(m_sceneId))
        , m_sceneRendererStateTracker(m_sceneId)
    {
        ramses::DisplayConfig displayConfig;
        displayConfig.setWaylandIviSurfaceID(0u);
        m_displayId = m_displayManager.createDisplay(displayConfig);
        m_displayManager.dispatchAndFlush();
        m_renderer.doOneLoop();
        m_displayManager.dispatchAndFlush();

        // flush at least once so that scene can be subscribed
        m_scene.flush();
    }

protected:
    void dispatchAndFlush()
    {
        expectSyncedSceneStates();
        m_displayManager.dispatchAndFlush(&m_sceneRendererStateTracker);
        expectSyncedSceneStates();
    }

    void expectSyncedSceneStates()
    {
        const auto lastDMState = m_displayManager.getSceneState(m_sceneId);
        const auto lastTrackedState = m_sceneRendererStateTracker.getLastState();

        switch (lastDMState)
        {
        case DisplayManager::ESceneState::GoingToSubscribed:
            EXPECT_TRUE(lastTrackedState == DisplayManager::ESceneState::Published || lastTrackedState == DisplayManager::ESceneState::Mapped);
            break;
        case DisplayManager::ESceneState::GoingToMapped:
            EXPECT_TRUE(lastTrackedState == DisplayManager::ESceneState::Subscribed || lastTrackedState == DisplayManager::ESceneState::Rendered);
            break;
        case DisplayManager::ESceneState::GoingToRendered:
            EXPECT_EQ(DisplayManager::ESceneState::Mapped, lastTrackedState);
            break;
        default:
            EXPECT_EQ(lastTrackedState, lastDMState);
            break;
        }
    }

    ramses::RamsesFramework m_framework;
    ramses::RamsesRenderer m_renderer;
    ramses::RamsesClient m_client;

    DisplayManager m_displayManager;
    ramses::IRendererEventHandler& m_displayManagerEventHandler;

    ramses::displayId_t m_displayId;
    const ramses::sceneId_t m_sceneId;
    ramses::Scene& m_scene;

    SceneStateTracker m_sceneRendererStateTracker;
};

TEST_F(ADisplayManagerWithRenderer, willShowSceneWhenPublished)
{
    m_displayManager.showSceneOnDisplay(m_sceneId, m_displayId);
    m_scene.publish(ramses::EScenePublicationMode_LocalOnly);

    for (int i = 0; i < 6; ++i)
    {
        m_renderer.doOneLoop();
        dispatchAndFlush();
    }
}

/////////
// Combinatorial tests where unpublish and republish come in every possible time during getting scene to SHOWN
/////////
class ADisplayManagerWithRenderer_ShowWithUnpublish : public ADisplayManagerWithRenderer, public WithParamInterface<int>
{
public:
    static constexpr int NumLoopsToReachTargetState = 12; // little more than actually needed
    static constexpr int NumStepsToReachTargetState = NumLoopsToReachTargetState * 2; // 2 steps per loop
    static constexpr int MaxNumStepsRepublishComesAfter = 4;
    static constexpr int NumCombinations = (NumStepsToReachTargetState - 5) * MaxNumStepsRepublishComesAfter; // can unpublish at any step, with room to process (-5), and encodes when publish comes (*MaxNumStepsRepublishComesAfter)

    int GetStepToUnpublish() const
    {
        return GetParam() / MaxNumStepsRepublishComesAfter;
    }

    int GetStepToRepublish() const
    {
        // either at same step or one step
        return GetStepToUnpublish() + GetParam() % MaxNumStepsRepublishComesAfter;
    }
};

// INSTANTIATE_TEST_CASE_P(Combinations, ADisplayManagerWithRenderer_ShowWithUnpublish, Range(0, ADisplayManagerWithRenderer_ShowWithUnpublish::NumCombinations));
// TODO reenable 8-12 after fix!!!
INSTANTIATE_TEST_CASE_P(Combinations1, ADisplayManagerWithRenderer_ShowWithUnpublish, Range(0, 8));
INSTANTIATE_TEST_CASE_P(Combinations2, ADisplayManagerWithRenderer_ShowWithUnpublish, Range(12, ADisplayManagerWithRenderer_ShowWithUnpublish::NumCombinations));

TEST_P(ADisplayManagerWithRenderer_ShowWithUnpublish, willGetSceneToShownWithUnpublishAtAnyStep)
{
    m_displayManager.showSceneOnDisplay(m_sceneId, m_displayId);
    m_scene.publish(ramses::EScenePublicationMode_LocalOnly);

    int step = 0;
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToUnpublish())
            m_scene.unpublish();
        if (step == GetStepToRepublish())
            m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
        m_renderer.doOneLoop();
        ++step;

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
        if (step == GetStepToUnpublish())
            m_scene.unpublish();
        if (step == GetStepToRepublish())
            m_scene.publish(ramses::EScenePublicationMode_LocalOnly);
        dispatchAndFlush();
        ++step;
    }
    // at this point scene was unpublished and republished
    ASSERT_TRUE(m_sceneRendererStateTracker.wasUnpublishedAndRepublished());

    // execute few more loops to allow DM to get to target state
    for (int i = 0; i < NumLoopsToReachTargetState; ++i)
    {
        m_renderer.doOneLoop();
        dispatchAndFlush();
    }

    // expect shown and synced
    EXPECT_EQ(DisplayManager::ESceneState::Rendered, m_displayManager.getSceneState(m_sceneId));
    expectSyncedSceneStates();
}
