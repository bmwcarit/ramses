//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "impl/RendererSceneControlImpl.h"
#include "ramses/framework/RamsesFramework.h"
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/RamsesRendererImpl.h"
#include "PlatformFactoryMock.h"
#include <memory>

using namespace testing;

namespace ramses::internal
{
    class RendererSceneStateTracker final : public RendererSceneControlEventHandlerEmpty
    {
    public:
        void sceneStateChanged(sceneId_t /*sceneId*/, RendererSceneState state) override
        {
            m_lastState = state;
        }

        RendererSceneState m_lastState = RendererSceneState::Unavailable;
    };

    class RamsesClientAndRendererWithScene
    {
    public:
        RamsesClientAndRendererWithScene()
            : m_renderer(*m_framework.createRenderer({}))
            , m_client(*m_framework.createClient("client"))
            , m_sceneControl(*m_renderer.getSceneControlAPI())
            , m_scene(CreateScene(m_client, m_sceneId))
        {
            m_renderer.impl().getDisplayDispatcher().injectPlatformFactory(std::make_unique<PlatformFactoryNiceMock>());
            m_displayId = m_renderer.createDisplay({});

            m_renderer.flush();
            m_renderer.doOneLoop();

            // flush at least once so that scene can be subscribed
            m_scene.flush();

            // set default mapping
            m_sceneControl.setSceneMapping(m_sceneId, m_displayId);
        }

        RamsesFramework m_framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        RamsesRenderer& m_renderer;
        RamsesClient& m_client;
        RendererSceneControl& m_sceneControl;

        displayId_t m_displayId;
        const sceneId_t m_sceneId{ 33u };
        ramses::Scene& m_scene;

    private:
        static ramses::Scene& CreateScene(RamsesClient& client, sceneId_t sceneId)
        {
            // Run tests in remote compatible mode, i.e. using shadow copy scene, so that scene does not have to be flushed
            // at every re-subscription.
            // Alternative is to flush scene on every state change (or at least re-subscription) before render loop.
            SceneConfig config{sceneId, EScenePublicationMode::LocalAndRemote};
            return *client.createScene(config);
        }
    };

    class ARendererSceneControlWithRenderer : public Test
    {
    public:
        void SetUp() override
        {
            ramsesClientRenderer->m_scene.publish(EScenePublicationMode::LocalOnly);
        }

        void TearDown() override
        {
            ramsesClientRenderer->m_scene.unpublish();
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }

        static void SetUpTestSuite()
        {
            ramsesClientRenderer = std::make_unique<RamsesClientAndRendererWithScene>();
        }

        static void TearDownTestSuite()
        {
            ramsesClientRenderer.reset();
        }

    protected:
        void expectSceneState(RendererSceneState state)
        {
            EXPECT_EQ(state, m_sceneStateTracker.m_lastState);
        }

        RendererSceneStateTracker m_sceneStateTracker;

        // static to avoid reinits for all the test cases
        static std::unique_ptr<RamsesClientAndRendererWithScene> ramsesClientRenderer;

        static constexpr int StartState = static_cast<int>(RendererSceneState::Available);
        static constexpr int NumStates = static_cast<int>(RendererSceneState::Rendered) + 1 - StartState;
        static constexpr int NumLoopsToReachRenderedTargetState = 6; // from published to rendered
    };

    std::unique_ptr<RamsesClientAndRendererWithScene> ARendererSceneControlWithRenderer::ramsesClientRenderer;

    TEST_F(ARendererSceneControlWithRenderer, willShowSceneWhenPublished)
    {
        ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, RendererSceneState::Rendered);
        ramsesClientRenderer->m_sceneControl.flush();

        for (int i = 0; i < NumLoopsToReachRenderedTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }

        EXPECT_EQ(RendererSceneState::Rendered, m_sceneStateTracker.m_lastState);
    }

    TEST_F(ARendererSceneControlWithRenderer, doesNotAttemptToReachTargetStateAfterUnpublishAndRepublish)
    {
        ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, RendererSceneState::Rendered);
        ramsesClientRenderer->m_sceneControl.flush();
        for (int i = 0; i < NumLoopsToReachRenderedTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }
        EXPECT_EQ(RendererSceneState::Rendered, m_sceneStateTracker.m_lastState);

        ramsesClientRenderer->m_scene.unpublish();

        for (int i = 0; i < NumLoopsToReachRenderedTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }
        EXPECT_EQ(RendererSceneState::Unavailable, m_sceneStateTracker.m_lastState);

        ramsesClientRenderer->m_scene.publish(EScenePublicationMode::LocalOnly);

        for (int i = 0; i < NumLoopsToReachRenderedTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }
        EXPECT_EQ(RendererSceneState::Available, m_sceneStateTracker.m_lastState);
    }

    /////////
    // Combinatorial tests where every combination of: target state is set and new target state is set at some point
    /////////
    class ARendererSceneControlWithRenderer_TargetToTarget : public ARendererSceneControlWithRenderer, public WithParamInterface<int>
    {
    public:
        static constexpr int NumLoopsToReachTargetState = NumLoopsToReachRenderedTargetState;
        static constexpr int NumStepsToReachTargetState = NumLoopsToReachTargetState * 2; // 2 steps per loop
        static constexpr int NumCombinations = NumStepsToReachTargetState * NumStates * NumStates; // can switch to new state at any step

        [[nodiscard]] static RendererSceneState GetTargetState1()
        {
            return RendererSceneState(StartState + (GetParam() / NumStepsToReachTargetState) / NumStates);
        }

        [[nodiscard]] static RendererSceneState GetTargetState2()
        {
            return RendererSceneState(StartState + (GetParam() / NumStepsToReachTargetState) % NumStates);
        }

        [[nodiscard]] static int GetStepToSwitchState()
        {
            return GetParam() % NumStepsToReachTargetState;
        }
    };

    INSTANTIATE_TEST_SUITE_P(TargetToTarget, ARendererSceneControlWithRenderer_TargetToTarget, Range(0, ARendererSceneControlWithRenderer_TargetToTarget::NumCombinations));

    TEST_P(ARendererSceneControlWithRenderer_TargetToTarget, switchFromTargetStateToTargetStateAtEveryStep)
    {
        LOG_INFO(CONTEXT_RENDERER, "Testing switch from target state " << EnumToString(GetTargetState1()) << " to target state " << EnumToString(GetTargetState2()) << " at step " << GetStepToSwitchState());

        ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState1());
        ramsesClientRenderer->m_sceneControl.flush();

        int step = 0;
        for (int i = 0; i < NumLoopsToReachTargetState; ++i)
        {
            LOG_INFO(CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
            if (step == GetStepToSwitchState())
            {
                ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState2());
                ramsesClientRenderer->m_sceneControl.flush();
            }
            ramsesClientRenderer->m_renderer.doOneLoop();
            ++step;

            LOG_INFO(CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
            if (step == GetStepToSwitchState())
            {
                ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState2());
                ramsesClientRenderer->m_sceneControl.flush();
            }
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
            ++step;
        }

        // execute few more loops to get to target state
        for (int i = 0; i < NumLoopsToReachTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }

        // expect target state reached
        EXPECT_EQ(GetTargetState2(), m_sceneStateTracker.m_lastState);
    }

    /////////
    // Combinatorial tests where unpublish and republish come in every possible time during getting scene to all possible states
    /////////
    class ARendererSceneControlWithRenderer_ReachTargetStateWithUnpublish : public ARendererSceneControlWithRenderer, public WithParamInterface<int>
    {
    public:
        static constexpr int MaxNumStepsRepublishComesAfter = 4;
        static constexpr int NumLoopsToReachTargetState = NumLoopsToReachRenderedTargetState + MaxNumStepsRepublishComesAfter; // add room for re/publish combinations
        static constexpr int NumStepsToReachTargetState = NumLoopsToReachTargetState * 2; // 2 steps per loop
        static constexpr int NumCombinations = (NumStepsToReachTargetState - 5) * MaxNumStepsRepublishComesAfter * NumStates; // can unpublish at any step, with room to process (-5), and encodes when publish comes (*MaxNumStepsRepublishComesAfter) and target state

        [[nodiscard]] static RendererSceneState GetTargetState()
        {
            return RendererSceneState(StartState + (GetParam() % NumStates));
        }

        [[nodiscard]] static int GetStepToUnpublish()
        {
            return (GetParam() / NumStates) / MaxNumStepsRepublishComesAfter;
        }

        [[nodiscard]] static int GetStepToRepublish()
        {
            // either at same step or one step
            return GetStepToUnpublish() + (GetParam() / NumStates) % MaxNumStepsRepublishComesAfter;
        }
    };

    INSTANTIATE_TEST_SUITE_P(ReachTargetStateWithUnpublish, ARendererSceneControlWithRenderer_ReachTargetStateWithUnpublish, Range(0, ARendererSceneControlWithRenderer_ReachTargetStateWithUnpublish::NumCombinations));

    TEST_P(ARendererSceneControlWithRenderer_ReachTargetStateWithUnpublish, unpublishAtEveryStepWhileReachingEveryState)
    {
        LOG_INFO(CONTEXT_RENDERER, "Testing unpublish from target state " << EnumToString(GetTargetState()) << " at step " << GetStepToUnpublish() << " and re-publish at step " << GetStepToRepublish());

        ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, GetTargetState());
        ramsesClientRenderer->m_sceneControl.flush();

        int step = 0;
        for (int i = 0; i < NumLoopsToReachTargetState; ++i)
        {
            LOG_INFO(CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
            if (step == GetStepToUnpublish())
                ramsesClientRenderer->m_scene.unpublish();
            if (step == GetStepToRepublish())
                ramsesClientRenderer->m_scene.publish(EScenePublicationMode::LocalOnly);
            ramsesClientRenderer->m_renderer.doOneLoop();
            ++step;

            LOG_INFO(CONTEXT_RENDERER, "LOOP: " << i << "  STEP: " << step);
            if (step == GetStepToUnpublish())
                ramsesClientRenderer->m_scene.unpublish();
            if (step == GetStepToRepublish())
                ramsesClientRenderer->m_scene.publish(EScenePublicationMode::LocalOnly);
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
            ++step;
        }

        // execute few more loops
        for (int i = 0; i < NumLoopsToReachTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }
        EXPECT_EQ(RendererSceneState::Available, m_sceneStateTracker.m_lastState);

        // test that rendered state can be reached again in new cycle
        ramsesClientRenderer->m_sceneControl.setSceneState(ramsesClientRenderer->m_sceneId, RendererSceneState::Rendered);
        ramsesClientRenderer->m_sceneControl.flush();

        // execute few more loops to get to rendered state
        for (int i = 0; i < NumLoopsToReachTargetState; ++i)
        {
            ramsesClientRenderer->m_renderer.doOneLoop();
            ramsesClientRenderer->m_sceneControl.dispatchEvents(m_sceneStateTracker);
        }

        // expect rendered state
        EXPECT_EQ(RendererSceneState::Rendered, m_sceneStateTracker.m_lastState);
    }
}
