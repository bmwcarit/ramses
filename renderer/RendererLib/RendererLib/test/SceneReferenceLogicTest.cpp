//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererLib/SceneReferenceLogic.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/SceneReferenceOwnership.h"
#include "RendererSceneStateControlMock.h"
#include "RendererSceneControlLogicMock.h"
#include "RendererSceneUpdaterMock.h"
#include "RendererSceneEventSenderMock.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"
#include "Utils/ThreadLocalLog.h"

using namespace testing;

namespace ramses_internal
{
    class ASceneReferenceLogic : public Test
    {
    public:
        ASceneReferenceLogic()
            : m_scenes(m_eventCollector)
            , m_logic(m_scenes, m_sceneLogic, m_sceneUpdater, m_eventSender, m_ownership)
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

        virtual void SetUp() override
        {
            SceneAllocateHelper masterScene1{ m_scenes.createScene(SceneInfo{ MasterSceneId1 }) };
            masterScene1.allocateSceneReference(RefSceneId11, RefSceneHandle11);
            masterScene1.allocateSceneReference(RefSceneId12, RefSceneHandle12);
            SceneAllocateHelper masterScene2{ m_scenes.createScene(SceneInfo{ MasterSceneId2 }) };
            masterScene2.allocateSceneReference(RefSceneId21, RefSceneHandle21);
            masterScene2.allocateSceneReference(RefSceneId22, RefSceneHandle22);

            m_scenes.createScene(SceneInfo{ RefSceneId11 });
            m_scenes.createScene(SceneInfo{ RefSceneId12 });
            m_scenes.createScene(SceneInfo{ RefSceneId21 });
            m_scenes.createScene(SceneInfo{ RefSceneId22 });

            // ignore this unless overridden by test
            EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; });
        }

    protected:
        void updateLogicAndVerifyExpectations()
        {
            m_logic.update();
            Mock::VerifyAndClearExpectations(&m_sceneLogic);
            Mock::VerifyAndClearExpectations(&m_sceneUpdater);

            // ignore this unless overridden by test
            EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; });
        }

        void expectSceneRefEventSent(SceneId refScene, SceneId expectedMasterScene)
        {
            RendererEventVector events;
            RendererEvent event{ ERendererEventType::SceneStateChanged };
            event.sceneId = refScene;
            event.state = RendererSceneState::Ready;
            events.push_back(event);

            // ref scene event will be extracted and sent
            EXPECT_CALL(m_eventSender, sendSceneStateChanged(expectedMasterScene, refScene, RendererSceneState::Ready));
            m_logic.extractAndSendSceneReferenceEvents(events);
        }

        RendererEventCollector m_eventCollector;
        RendererScenes m_scenes;
        StrictMock<RendererSceneControlLogicMock> m_sceneLogic;
        StrictMock<RendererSceneUpdaterMock> m_sceneUpdater;
        StrictMock<RendererSceneEventSenderMock> m_eventSender;
        SceneReferenceOwnership m_ownership;
        SceneReferenceLogic m_logic;

        static constexpr SceneId MasterSceneId1{ 123 };
        static constexpr SceneId MasterSceneId2{ 124 };
        static constexpr SceneId RefSceneId11{ 125 };
        static constexpr SceneId RefSceneId12{ 126 };
        static constexpr SceneId RefSceneId21{ 127 };
        static constexpr SceneId RefSceneId22{ 128 };
        static constexpr SceneReferenceHandle RefSceneHandle11{ 1 };
        static constexpr SceneReferenceHandle RefSceneHandle12{ 2 };
        static constexpr SceneReferenceHandle RefSceneHandle21{ 3 };
        static constexpr SceneReferenceHandle RefSceneHandle22{ 4 };
    };

    constexpr SceneId ASceneReferenceLogic::MasterSceneId1;
    constexpr SceneId ASceneReferenceLogic::MasterSceneId2;
    constexpr SceneId ASceneReferenceLogic::RefSceneId11;
    constexpr SceneId ASceneReferenceLogic::RefSceneId12;
    constexpr SceneId ASceneReferenceLogic::RefSceneId21;
    constexpr SceneId ASceneReferenceLogic::RefSceneId22;
    constexpr SceneReferenceHandle ASceneReferenceLogic::RefSceneHandle11;
    constexpr SceneReferenceHandle ASceneReferenceLogic::RefSceneHandle12;
    constexpr SceneReferenceHandle ASceneReferenceLogic::RefSceneHandle21;
    constexpr SceneReferenceHandle ASceneReferenceLogic::RefSceneHandle22;

    TEST_F(ASceneReferenceLogic, updatesScansScenesForReferencesToUpdate)
    {
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId21, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId22, _, _, _));
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId11));
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId12));
        EXPECT_EQ(MasterSceneId2, m_ownership.getSceneOwner(RefSceneId21));
        EXPECT_EQ(MasterSceneId2, m_ownership.getSceneOwner(RefSceneId22));
    }

    TEST_F(ASceneReferenceLogic, cannotFindMasterAfterRefSceneReleased)
    {
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle12);
        EXPECT_FALSE(m_ownership.getSceneOwner(RefSceneId12).isValid());
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);
        EXPECT_FALSE(m_ownership.getSceneOwner(RefSceneId11).isValid());
    }

    TEST_F(ASceneReferenceLogic, executesActionForMasterScene)
    {
        static constexpr SceneReferenceAction linkAction1{ SceneReferenceActionType::LinkData, {}, DataSlotId{1}, RefSceneHandle11, DataSlotId{2} };
        m_logic.addActions(MasterSceneId1, { linkAction1 });

        EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(RefSceneId11, linkAction1.providerId, MasterSceneId1, linkAction1.consumerId));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, executesMultipleActionsForMultipleMasterScenes)
    {
        static constexpr SceneReferenceAction linkAction1{ SceneReferenceActionType::LinkData, {}, DataSlotId{1}, RefSceneHandle11, DataSlotId{2} };
        static constexpr SceneReferenceAction linkAction2{ SceneReferenceActionType::LinkData, RefSceneHandle12, DataSlotId{3}, {}, DataSlotId{4} };
        static constexpr SceneReferenceAction unlinkAction1{ SceneReferenceActionType::UnlinkData, {}, DataSlotId{5}, {}, {} };
        static constexpr SceneReferenceAction unlinkAction2{ SceneReferenceActionType::UnlinkData, RefSceneHandle22, DataSlotId{6}, {}, {} };
        m_logic.addActions(MasterSceneId1, { linkAction1, linkAction2, unlinkAction1 });
        m_logic.addActions(MasterSceneId2, { unlinkAction1, unlinkAction2 });

        EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(RefSceneId11, linkAction1.providerId, MasterSceneId1, linkAction1.consumerId));
        EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(MasterSceneId1, linkAction2.providerId, RefSceneId12, linkAction2.consumerId));
        EXPECT_CALL(m_sceneUpdater, handleDataUnlinkRequest(MasterSceneId1, unlinkAction1.consumerId));
        EXPECT_CALL(m_sceneUpdater, handleDataUnlinkRequest(MasterSceneId2, unlinkAction1.consumerId));
        EXPECT_CALL(m_sceneUpdater, handleDataUnlinkRequest(RefSceneId22, unlinkAction2.consumerId));
        updateLogicAndVerifyExpectations();

        // actions cleared, no more expectations
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, requestsStateChangeForReferencedScene)
    {
        // simulate master scene Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, delaysRequestStateChangeForReferencedSceneUntilMasterSceneAtLeastAtSameStateLevel)
    {
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        // no expecation, master scene is unavailable
        updateLogicAndVerifyExpectations();

        // simulate master scene Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, reactsToMasterSceneStateChangeSoThatReferencedScenesNeverHaveHigherStateThanMaster_fromAvailableUp)
    {
        // simulate master scene Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate master scene state Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // ref scenes now can advance to Ready as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state to Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        // ref scene requested to be Rendered can now advance as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Rendered));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, reactsToMasterSceneStateChangeSoThatReferencedScenesNeverHaveHigherStateThanMaster_fromRenderedDown)
    {
        // simulate master scene Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Rendered));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Rendered; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state dropped to Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // previously rendered ref scene dropped to Ready as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state dropped to Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        // both ref scenes drop to Available as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referencedScenesInheritAssignmentInformationFromMasterScene)
    {
        constexpr OffscreenBufferHandle ob{ 3 };

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob, -13));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob, -13));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referencedScenesGetUpdatedBufferAssignmentFromMasterScene)
    {
        constexpr OffscreenBufferHandle ob1{ 4 };
        constexpr OffscreenBufferHandle ob2{ 5 };

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob1;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, -13));
        updateLogicAndVerifyExpectations();

        // from now on both master and ref scenes have same mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([&](auto, auto& state, auto& ob_, auto& renderOrder)
        {
            state = RendererSceneState::Unavailable; // scene state is irrelevant for this test
            ob_ = ob1;
            renderOrder = -13;
        }));
        // do not clear expectations
        m_logic.update();

        // simulate master scene has new buffer set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob2;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob2, -13));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob2, -13));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referencedScenesGetUpdatedRenderOrderFromMasterScene)
    {
        constexpr OffscreenBufferHandle ob{ 4 };

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob, -13));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob, -13));
        updateLogicAndVerifyExpectations();

        // from now on both master and ref scenes have same mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([&](auto, auto& state, auto& ob_, auto& renderOrder)
        {
            state = RendererSceneState::Unavailable; // scene state is irrelevant for this test
            ob_ = ob;
            renderOrder = -13;
        }));
        // do not clear expectations
        m_logic.update();

        // simulate master scene has new render order set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob;
            renderOrder = -3;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob, -3));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob, -3));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, setsRenderOrderForReferencedScene)
    {
        constexpr OffscreenBufferHandle ob1{ 4 };

        // first set mapping information for master scene which will trigger setting mapping info on its references
        // scene render order can be set only after mapping info available
        {
            // simulate master scene has mapping set
            EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
            {
                ob_ = ob1;
                renderOrder = -13;
            }));

            EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13)); // requested render order is relative to master
            EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, -13)); // requested render order is relative to master
            m_logic.update();

            // next time a ref scene is queried for mapping info use the last requested values
            EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
            {
                ob_ = ob1;
                renderOrder = -13;
            }));
            EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
            {
                ob_ = ob1;
                renderOrder = -13;
            }));
        }

        // set render order
        m_scenes.getScene(MasterSceneId1).setSceneReferenceRenderOrder(RefSceneHandle11, -666);
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13 - 666));
        m_logic.update();

        // set another render order
        m_scenes.getScene(MasterSceneId1).setSceneReferenceRenderOrder(RefSceneHandle11, 1234);
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13 + 1234));
        m_logic.update();

        // simulate master scene render order changed
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& ob_, auto& renderOrder)
        {
            ob_ = ob1;
            renderOrder = 123;
        }));

        // will update both referenced scenes' render order
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, 123 + 1234));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, 123 + 0));
        m_logic.update();
    }

    TEST_F(ASceneReferenceLogic, requestsAvailableStateForAllReferencedScenesOfDestroyedMasterScene)
    {
        // simulate master scenes Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // request ref scenes ready
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(RefSceneHandle21, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(RefSceneHandle22, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // now report every scene as requested ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // destroy one master
        m_scenes.destroyScene(MasterSceneId2);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        // destroy another master
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, canHandleMasterWithRefsAfterItWasDestroyedAndMadeAvailableAgain)
    {
        // simulate master scene Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // request ref scenes Ready
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // destroy master
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // recreate master with refs
        SceneAllocateHelper masterScene1{ m_scenes.createScene(SceneInfo{ MasterSceneId1 }) };
        masterScene1.allocateSceneReference(RefSceneId11, RefSceneHandle11);
        masterScene1.allocateSceneReference(RefSceneId12, RefSceneHandle12);
        updateLogicAndVerifyExpectations();
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId11));
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId12));

        // simulate master scene Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // request ref scenes available in new cycle
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // destroy master second time
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, canHandleMasterWithRefsAfterItWasDestroyedAndMadeAvailableAgain_whenRefsDidNotArriveYet)
    {
        // refs will never arrive
        m_scenes.destroyScene(RefSceneId11);
        m_scenes.destroyScene(RefSceneId12);

        // simulate master scene Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // request ref scenes Ready
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // destroy master
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // recreate master with refs
        SceneAllocateHelper masterScene1{ m_scenes.createScene(SceneInfo{ MasterSceneId1 }) };
        masterScene1.allocateSceneReference(RefSceneId11, RefSceneHandle11);
        masterScene1.allocateSceneReference(RefSceneId12, RefSceneHandle12);
        updateLogicAndVerifyExpectations();
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId11));
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId12));

        // simulate master scene Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // request ref scenes available in new cycle
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // now report ref scenes as requested Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // destroy master second time
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, doesNotSendNorExtractPublishEventOfReferencedScene)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEvent event{ ERendererEventType::ScenePublished };
        event.sceneId = RefSceneId12;
        RendererEventVector events{ event };

        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::ScenePublished, events[0].eventType);
        EXPECT_EQ(RefSceneId12, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, doesNotSendNorExtractEventsOfUninterestingType)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::DisplayCreated }, { ERendererEventType::WindowKeyEvent }, { ERendererEventType::ReadPixelsFromFramebuffer } };
        RendererEventVector eventsOriginal = events;

        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(eventsOriginal.size(), events.size());
        for (size_t i = 0u; i < events.size(); ++i)
            EXPECT_EQ(eventsOriginal[i].eventType, events[i].eventType);
    }

    TEST_F(ASceneReferenceLogic, discardsSceneEventsOfIrrelevantTypesComingFromReferencedScenes)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // 2 sets of same type of events but from different sources

        // these come from referenced scene and will be discarded
        RendererEventVector eventsToDiscard
        {
            { ERendererEventType::SceneDataSlotProviderCreated },
            { ERendererEventType::SceneDataSlotProviderDestroyed },
            { ERendererEventType::SceneDataSlotConsumerCreated },
            { ERendererEventType::SceneDataSlotConsumerDestroyed }
        };
        eventsToDiscard[0].providerSceneId = eventsToDiscard[1].providerSceneId = RefSceneId12;
        eventsToDiscard[2].consumerSceneId = eventsToDiscard[3].consumerSceneId = RefSceneId21;

        // these come from master scene and will be kept
        RendererEventVector eventsToKeep = eventsToDiscard;
        eventsToKeep[0].providerSceneId = eventsToKeep[1].providerSceneId = MasterSceneId1;
        eventsToKeep[2].consumerSceneId = eventsToKeep[3].consumerSceneId = MasterSceneId2;

        // concatenate and process
        RendererEventVector events = eventsToDiscard;
        events.insert(events.end(), eventsToKeep.cbegin(), eventsToKeep.cend());
        m_logic.extractAndSendSceneReferenceEvents(events);

        // only those meant to be kept are left in processed list
        ASSERT_EQ(eventsToKeep.size(), events.size());
        for (size_t i = 0; i < eventsToKeep.size(); ++i)
        {
            EXPECT_EQ(eventsToKeep[i].eventType, events[i].eventType);
            EXPECT_EQ(eventsToKeep[i].providerSceneId, events[i].providerSceneId);
            EXPECT_EQ(eventsToKeep[i].consumerSceneId, events[i].consumerSceneId);
        }
    }

    TEST_F(ASceneReferenceLogic, extractsButDoesNotSendSceneRefEventsOfUnusedDataLinkingTypes)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events
        {
            { ERendererEventType::SceneDataBufferLinked },
            { ERendererEventType::SceneDataBufferLinkFailed }, { ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange },
            { ERendererEventType::DisplayCreated }, // will be kept
        };
        // all of them have ref scene as consumer scene
        for (auto& evt : events)
            evt.consumerSceneId = RefSceneId12;
        RendererEventVector eventsOriginal = events;

        m_logic.extractAndSendSceneReferenceEvents(events);

        // display creation event was the only one kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::DisplayCreated, events[0].eventType);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneStateChange)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneStateChanged };
        event.sceneId = RefSceneId21;
        event.state = RendererSceneState::Ready;
        events.push_back(event);

        // same event but for not referenced scene
        event.sceneId = MasterSceneId1;
        events.push_back(event);

        // ref scene event will be extracted and sent
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId2, RefSceneId21, RendererSceneState::Ready));
        m_logic.extractAndSendSceneReferenceEvents(events);

        // non-ref scene event will is kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneStateChanged, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(RendererSceneState::Ready, events[0].state);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneStateChange_masterSceneAlreadyDestroyed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        m_scenes.destroyScene(MasterSceneId2);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneStateChanged };
        event.sceneId = RefSceneId21;
        event.state = RendererSceneState::Ready;
        events.push_back(event);

        // ref scene event will be extracted and sent
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId2, RefSceneId21, RendererSceneState::Ready));
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, extractsEventOfReferencedScene_SceneFlushed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        constexpr SceneVersionTag version{ 123 };
        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneFlushed };
        event.sceneId = RefSceneId21;
        event.sceneVersionTag = version;
        events.push_back(event);

        // same event but for not referenced scene
        event.sceneId = MasterSceneId1;
        events.push_back(event);

        // ref scene event will be extracted
        m_logic.extractAndSendSceneReferenceEvents(events);
        // event not sent because flush notification not enabled (default state)

        // non-ref scene event will is kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneFlushed, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(version, events[0].sceneVersionTag);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneFlushed_ifNotificationEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());
        // enable flush notifications for one of the scenes
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, false);
        updateLogicAndVerifyExpectations();

        constexpr SceneVersionTag version{ 123 };
        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneFlushed };
        event.sceneId = RefSceneId21;
        event.sceneVersionTag = version;
        events.push_back(event);

        // same event second referenced scene
        event.sceneId = RefSceneId22;
        events.push_back(event);

        // ref scene event will be extracted for both
        // but sent only to the one with enabled notifications
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId21, version));
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataLinked)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneDataLinked };
        // master -> ref
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = RefSceneId11;
        event.providerdataId = DataSlotId{ 1 };
        event.consumerdataId = DataSlotId{ 2 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId1, MasterSceneId1, DataSlotId{ 1 }, RefSceneId11, DataSlotId{ 2 }, true));

        // ref -> master
        event.providerSceneId = RefSceneId21;
        event.consumerSceneId = MasterSceneId2;
        event.providerdataId = DataSlotId{ 3 };
        event.consumerdataId = DataSlotId{ 4 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId2, RefSceneId21, DataSlotId{ 3 }, MasterSceneId2, DataSlotId{ 4 }, true));

        // ref -> ref
        event.providerSceneId = RefSceneId11;
        event.consumerSceneId = RefSceneId12;
        event.providerdataId = DataSlotId{ 5 };
        event.consumerdataId = DataSlotId{ 6 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId1, RefSceneId11, DataSlotId{ 5 }, RefSceneId12, DataSlotId{ 6 }, true));

        // master -> master
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = MasterSceneId2;
        event.providerdataId = DataSlotId{ 7 };
        event.consumerdataId = DataSlotId{ 8 };
        events.push_back(event);
        // expect no call

        m_logic.extractAndSendSceneReferenceEvents(events);

        // only master -> master event was kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataLinked, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(DataSlotId{ 7 }, events[0].providerdataId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataLinkFailed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneDataLinkFailed };
        // master -> ref
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = RefSceneId11;
        event.providerdataId = DataSlotId{ 1 };
        event.consumerdataId = DataSlotId{ 2 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId1, MasterSceneId1, DataSlotId{ 1 }, RefSceneId11, DataSlotId{ 2 }, false));

        // ref -> master
        event.providerSceneId = RefSceneId21;
        event.consumerSceneId = MasterSceneId2;
        event.providerdataId = DataSlotId{ 3 };
        event.consumerdataId = DataSlotId{ 4 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId2, RefSceneId21, DataSlotId{ 3 }, MasterSceneId2, DataSlotId{ 4 }, false));

        // ref -> ref
        event.providerSceneId = RefSceneId11;
        event.consumerSceneId = RefSceneId12;
        event.providerdataId = DataSlotId{ 5 };
        event.consumerdataId = DataSlotId{ 6 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataLinked(MasterSceneId1, RefSceneId11, DataSlotId{ 5 }, RefSceneId12, DataSlotId{ 6 }, false));

        // master -> master
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = MasterSceneId2;
        event.providerdataId = DataSlotId{ 7 };
        event.consumerdataId = DataSlotId{ 8 };
        events.push_back(event);
        // expect no call

        m_logic.extractAndSendSceneReferenceEvents(events);

        // only master -> master event was kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataLinkFailed, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(DataSlotId{ 7 }, events[0].providerdataId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataUnlinked)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneDataUnlinked };
        // master -> ref
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = RefSceneId11;
        event.consumerdataId = DataSlotId{ 2 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId1, RefSceneId11, DataSlotId{ 2 }, true));

        // ref -> master
        event.providerSceneId = RefSceneId21;
        event.consumerSceneId = MasterSceneId2;
        event.consumerdataId = DataSlotId{ 4 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId2, MasterSceneId2, DataSlotId{ 4 }, true));

        // ref -> ref
        event.providerSceneId = RefSceneId11;
        event.consumerSceneId = RefSceneId12;
        event.consumerdataId = DataSlotId{ 6 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId1, RefSceneId12, DataSlotId{ 6 }, true));

        // master -> master
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = MasterSceneId2;
        event.consumerdataId = DataSlotId{ 8 };
        events.push_back(event);
        // expect no call

        m_logic.extractAndSendSceneReferenceEvents(events);

        // only master -> master event was kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataUnlinked, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataUnlinkFailed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType::SceneDataUnlinkFailed };
        // master -> ref
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = RefSceneId11;
        event.consumerdataId = DataSlotId{ 2 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId1, RefSceneId11, DataSlotId{ 2 }, false));

        // ref -> master
        event.providerSceneId = RefSceneId21;
        event.consumerSceneId = MasterSceneId2;
        event.consumerdataId = DataSlotId{ 4 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId2, MasterSceneId2, DataSlotId{ 4 }, false));

        // ref -> ref
        event.providerSceneId = RefSceneId11;
        event.consumerSceneId = RefSceneId12;
        event.consumerdataId = DataSlotId{ 6 };
        events.push_back(event);
        EXPECT_CALL(m_eventSender, sendDataUnlinked(MasterSceneId1, RefSceneId12, DataSlotId{ 6 }, false));

        // master -> master
        event.providerSceneId = MasterSceneId1;
        event.consumerSceneId = MasterSceneId2;
        event.consumerdataId = DataSlotId{ 8 };
        events.push_back(event);
        // expect no call

        m_logic.extractAndSendSceneReferenceEvents(events);

        // only master -> master event was kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneDataUnlinkFailed, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEnabledEventOfReferencedSceneAndEmitsAsMasterExpirationEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpirationMonitoringEnabled, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEventOfReferencedSceneAndEmitsAsMasterExpired)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(2u, events.size());
        // expiration enabled will come first as master was not yet reported as enabled
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);
        EXPECT_EQ(MasterSceneId1, events[1].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEnabledEventOfMasterSceneAndEmitsAsMasterExpirationEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpirationMonitoringEnabled, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEventOfMasterSceneAndEmitsAsMasterExpired)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpired, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(2u, events.size());
        // expiration enabled will come first as master was not yet reported as enabled
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);
        EXPECT_EQ(MasterSceneId1, events[1].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEventOfReferencedSceneAndEmitsAsMasterExpired_onlyOnceForFirstRefExpiration)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 }, { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);
        EXPECT_EQ(MasterSceneId1, events[1].sceneId);

        // simulate more expired scenes from same master
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId11 }, { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        // no more expirations reported
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationEventOfMasterSceneAndEmitsAsMasterExpired_onlyOnceForFirstExpiration)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType::SceneExpired, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);
        EXPECT_EQ(MasterSceneId1, events[1].sceneId);

        // simulate more expired ref scenes from same master
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId11 }, { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        // no more expirations reported
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationRecoveryEventOfReferencedSceneAndEmitsAsMasterRecovered)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate expired first
        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_EQ(2u, events.size());

        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationRecoveryEventOfMasterSceneAndEmitsAsMasterRecovered)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate expired first
        RendererEventVector events{ { ERendererEventType::SceneExpired, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_EQ(2u, events.size());

        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationRecoveryEventOfReferencedSceneAndEmitsAsMasterRecovered_onlyOnceAfterAllRefsRecovered)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate ref1 expired
        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());

        // ref2 recovered
        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());

        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // only now both refs recovered
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationRecoveryEventOfSceneAndEmitsAsMasterRecovered_onlyOnceAfterAllRefsAndMasterRecovered)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate master expired
        RendererEventVector events{ { ERendererEventType::SceneExpired, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());

        // ref2 recovered
        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());

        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // only now both ref and master recovered
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesExpirationRecoveryAndDisableEventOfSceneAndEmitsAsMasterRecoveredAndDisabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate master enabled for monitoring
        RendererEventVector events{ { ERendererEventType::SceneExpirationMonitoringEnabled, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // report expired
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpired, events[0].eventType);

        // simulate master disabled for monitoring
        events = RendererEventVector{ { ERendererEventType::SceneExpirationMonitoringDisabled, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // does not affect consolidated state, one of refs still expired
        EXPECT_TRUE(events.empty());

        // ref2 recovered
        events = RendererEventVector{ { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        // report recovered
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);

        // ref2 disabled for monitoring
        events = RendererEventVector{ { ERendererEventType::SceneExpirationMonitoringDisabled, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);

        // report disabled as all disabled for monitoring now
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringDisabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, treatsExpiredRefAsDisabledIfItBecomesUnavailable)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate ref1 expired
        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());

        // ref2 unavailable
        RendererEvent evt{ ERendererEventType::SceneStateChanged, RefSceneId12 };
        evt.state = RendererSceneState::Unavailable;
        events = { evt };
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId1, RefSceneId12, _));
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());

        // ref1 unavailable
        evt.sceneId = RefSceneId11;
        events = { evt };
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId1, RefSceneId11, _));
        m_logic.extractAndSendSceneReferenceEvents(events);
        // both refs gone, master reported as disabled for monitoring
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringDisabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, treatsExpiredRefAsDisabledIfItBecomesUnavailable_withMasterEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate master enabled for monitoring
        RendererEventVector events{ { ERendererEventType::SceneExpirationMonitoringEnabled, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);

        // simulate ref1 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpired, events[0].eventType);

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());

        // ref2 unavailable
        RendererEvent evt{ ERendererEventType::SceneStateChanged, RefSceneId12 };
        evt.state = RendererSceneState::Unavailable;
        events = { evt };
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId1, RefSceneId12, _));
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());

        // ref1 unavailable
        evt.sceneId = RefSceneId11;
        events = { evt };
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId1, RefSceneId11, _));
        m_logic.extractAndSendSceneReferenceEvents(events);
        // both refs gone, master reported as recovered, master itself is still enabled for monitoring
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, reportsMasterAsDisabledIfExpiredRefsReleased)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate ref1 expired
        RendererEventVector events{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        EXPECT_EQ(ERendererEventType::SceneExpired, events[1].eventType);
        updateLogicAndVerifyExpectations();

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());
        updateLogicAndVerifyExpectations();

        // release expired ref2
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle12);
        updateLogicAndVerifyExpectations();
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());
        EXPECT_FALSE(m_ownership.getSceneOwner(RefSceneId12).isValid());

        // release expired ref1
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);
        updateLogicAndVerifyExpectations();
        m_logic.extractAndSendSceneReferenceEvents(events);
        // both expired refs gone, there is nothing monitored anymore, master reported as disabled
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringDisabled, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_FALSE(m_ownership.getSceneOwner(RefSceneId11).isValid());
    }

    TEST_F(ASceneReferenceLogic, reportsMasterAsRecoveredIfExpiredRefsReleased_withMasterEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // enable master monitoring
        RendererEventVector events{ { ERendererEventType::SceneExpirationMonitoringEnabled, MasterSceneId1 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpirationMonitoringEnabled, events[0].eventType);
        updateLogicAndVerifyExpectations();

        // simulate ref1 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId11 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneExpired, events[0].eventType);
        updateLogicAndVerifyExpectations();

        // simulate ref2 expired
        events = RendererEventVector{ { ERendererEventType::SceneExpired, RefSceneId12 } };
        m_logic.extractAndSendSceneReferenceEvents(events);
        // already expired, no event
        ASSERT_TRUE(events.empty());
        updateLogicAndVerifyExpectations();

        // release expired ref2
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle12);
        updateLogicAndVerifyExpectations();
        m_logic.extractAndSendSceneReferenceEvents(events);
        // ref1 still expired, no event
        EXPECT_TRUE(events.empty());

        // release expired ref1
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);
        updateLogicAndVerifyExpectations();
        m_logic.extractAndSendSceneReferenceEvents(events);
        // both expired refs gone, master reported as recovered - it is still enabled
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType::SceneRecoveredFromExpiration, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, consumesAllAndEmitsNothingIfFullExpirationToRecoverCycleComeAtSameTime_confidenceTest)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // both refs enabled, expired, recovered and disabled at same time
        RendererEventVector events
        {
            { ERendererEventType::SceneExpirationMonitoringEnabled, RefSceneId11 }, { ERendererEventType::SceneExpirationMonitoringEnabled, RefSceneId12 },
            { ERendererEventType::SceneExpired, RefSceneId11 }, { ERendererEventType::SceneExpired, RefSceneId12 },
            { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId11 }, { ERendererEventType::SceneRecoveredFromExpiration, RefSceneId12 },
            { ERendererEventType::SceneExpirationMonitoringDisabled, RefSceneId11 }, { ERendererEventType::SceneExpirationMonitoringDisabled, RefSceneId12 }
        };
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, sendsFlushEventFirstTimeNotificationIsEnabledForRefSceneWithValidVersion)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate versioned flush applied previously for one of the referenced scenes
        m_scenes.getStagingInfo(RefSceneId22).lastAppliedVersionTag = SceneVersionTag{ 666 };
        updateLogicAndVerifyExpectations();

        // enable flush notifications for both of the scenes
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, true);
        // send event only for scene with previously applied flush with valid version
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId22, SceneVersionTag{ 666 }));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, sendsFlushEventFirstTimeNotificationIsEnabled_onlyOnceWhenEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate versioned flush applied previously for one of the referenced scenes
        m_scenes.getStagingInfo(RefSceneId22).lastAppliedVersionTag = SceneVersionTag{ 666 };
        updateLogicAndVerifyExpectations();

        // enable flush notifications for both of the scenes
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, true);
        // send event for scene with previously applied flush with valid version
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId22, SceneVersionTag{ 666 }));
        updateLogicAndVerifyExpectations();

        // no more events
        updateLogicAndVerifyExpectations();

        // enabling already enabled notifications does not trigger new event either
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, true);
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, sendsFlushEventFirstTimeNotificationIsEnabled_againAfterDisabledAndEnabled)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        // simulate versioned flush applied previously for one of the referenced scenes
        m_scenes.getStagingInfo(RefSceneId22).lastAppliedVersionTag = SceneVersionTag{ 666 };
        updateLogicAndVerifyExpectations();

        // enable flush notifications for both of the scenes
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, true);
        // send event for scene with previously applied flush with valid version
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId22, SceneVersionTag{ 666 }));
        updateLogicAndVerifyExpectations();

        // disable notifications
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, false);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, false);
        updateLogicAndVerifyExpectations();

        // simulate versioned flush applied previously for the other ref scene
        m_scenes.getStagingInfo(RefSceneId21).lastAppliedVersionTag = SceneVersionTag{ 555 };

        // re-enable flush notifications for both of the scenes
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle21, true);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceFlushNotifications(RefSceneHandle22, true);
        // send event for both ref scenes
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId21, SceneVersionTag{ 555 }));
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId22, SceneVersionTag{ 666 }));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referenceCanBeControlledUnderNewMasterWhenRereferenced)
    {
        // other scenes not relevant for this test, keep available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate original master and ref scene Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Rendered; }));
        m_logic.update();

        // release ref from original master
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);

        // expect nothing
        m_logic.update();

        // re-reference by new master
        SceneAllocateHelper otherMaster{ m_scenes.getScene(MasterSceneId2) };
        const auto otherRefHandle = otherMaster.allocateSceneReference(RefSceneId11);

        // ref scene is now under new master which did not request any state for it yet, i.e. default is available
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        m_logic.update();
        EXPECT_EQ(MasterSceneId2, m_ownership.getSceneOwner(RefSceneId11));

        // simulate new master Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // request ref ready from new master now
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(otherRefHandle, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        m_logic.update();

        expectSceneRefEventSent(RefSceneId11, MasterSceneId2);
    }

    TEST_F(ASceneReferenceLogic, referenceCanGetNewMasterWhileKeepingItsState)
    {
        // test handover from one master to another while READY, which is expected to be common use case, however would work in any other state

        // other scenes not relevant for this test, keep unavailable
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate original master and ref scene READY
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        m_logic.update();

        // release ref from original master
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);

        // expect nothing
        m_logic.update();

        // new master must already be ready before handover - simulate new master READY
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // re-reference by new master
        SceneAllocateHelper otherMaster{ m_scenes.getScene(MasterSceneId2) };
        const auto otherRefHandle = otherMaster.allocateSceneReference(RefSceneId11);
        // even if reference is already in READY, new master must explicitly request it again within same update loop (HL flush)
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(otherRefHandle, RendererSceneState::Ready);

        // expect nothing, i.e. ref stays in READY
        m_logic.update();
        EXPECT_EQ(MasterSceneId2, m_ownership.getSceneOwner(RefSceneId11));

        // request another state for ref from new master now
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(otherRefHandle, RendererSceneState::Available);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        m_logic.update();

        expectSceneRefEventSent(RefSceneId11, MasterSceneId2);
    }

    TEST_F(ASceneReferenceLogic, referenceCanBeControlledUnderNewMasterWhenRereferencedAfterOldMasterDestroyed)
    {
        // other scenes not relevant for this test, keep available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate original master and ref scene Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(RefSceneHandle21, RendererSceneState::Rendered);
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId21, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Rendered; }));
        m_logic.update();

        // destroy original master
        m_scenes.destroyScene(MasterSceneId2);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Available));
        m_logic.update();

        // re-reference by new master
        SceneAllocateHelper otherMaster{ m_scenes.getScene(MasterSceneId1) };
        const auto otherRefHandle = otherMaster.allocateSceneReference(RefSceneId21);

        // ref scene is now under new master which did not request any state for it yet, i.e. default is available
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Available));
        m_logic.update();
        EXPECT_EQ(MasterSceneId1, m_ownership.getSceneOwner(RefSceneId21));

        // simulate new master Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // request ref ready from new master now
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(otherRefHandle, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Ready));
        m_logic.update();

        expectSceneRefEventSent(RefSceneId21, MasterSceneId1);
    }

    TEST_F(ASceneReferenceLogic, referenceGetsAssignmentFromNewMasterWhenRereferenced)
    {
        // remove non-relevant ref scenes to simplify this test
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle12);
        m_scenes.getScene(MasterSceneId2).releaseSceneReference(RefSceneHandle21);
        m_scenes.getScene(MasterSceneId2).releaseSceneReference(RefSceneHandle22);

        constexpr OffscreenBufferHandle ob1{ 3 };
        constexpr OffscreenBufferHandle ob2{ 4 };
        constexpr int32_t renderOrder1{ 5 };
        constexpr int32_t renderOrder2{ 6 };

        // simulate original master and ref scene Rendered and mapped
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([&](auto, auto& targetState, auto& ob, auto& renderOrder)
        {
            targetState = RendererSceneState::Rendered;
            ob = ob1;
            renderOrder = renderOrder1;
        }));
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([&](auto, auto& targetState, auto& ob, auto& renderOrder)
        {
            targetState = RendererSceneState::Rendered;
            ob = ob1;
            renderOrder = renderOrder1;
        }));
        m_logic.update();

        // release ref from original master
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);

        // expect nothing
        m_logic.update();

        // re-reference by new master
        SceneAllocateHelper otherMaster{ m_scenes.getScene(MasterSceneId2) };
        const auto otherRefHandle = otherMaster.allocateSceneReference(RefSceneId11);

        // ref scene is now under new master which did not request any state for it yet, i.e. default is unavailable
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        // default buffer assignement will be used initially
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, OffscreenBufferHandle::Invalid(), 0));
        m_logic.update();

        // simulate new master Ready with different assignment parameters
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _)).WillRepeatedly(Invoke([&](auto, auto& targetState, auto& ob, auto& renderOrder)
        {
            targetState = RendererSceneState::Rendered;
            ob = ob2;
            renderOrder = renderOrder2;
        }));

        // request ref ready from new master now
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(otherRefHandle, RendererSceneState::Ready);
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob2, renderOrder2));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        m_logic.update();
    }

    TEST_F(ASceneReferenceLogic, ignoresActionsThatArriveAfterReferenceReleased)
    {
        // other scenes not relevant for this test, keep available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate original master and ref scene Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Rendered; }));
        m_logic.update();

        // add some action
        static constexpr SceneReferenceAction linkAction1{ SceneReferenceActionType::LinkData, {}, DataSlotId{1}, RefSceneHandle11, DataSlotId{2} };
        static constexpr SceneReferenceAction linkAction2{ SceneReferenceActionType::LinkData, RefSceneHandle11, DataSlotId{1}, {}, DataSlotId{2} };
        static constexpr SceneReferenceAction linkAction3{ SceneReferenceActionType::LinkData, RefSceneHandle12, DataSlotId{1}, {}, DataSlotId{2} };
        m_logic.addActions(MasterSceneId1, { linkAction1, linkAction2, linkAction3 });

        // release ref from original master
        m_scenes.getScene(MasterSceneId1).releaseSceneReference(RefSceneHandle11);

        // only action remaining is for the ref that was not released, others are ignored as its ref does not have owner anymore
        EXPECT_CALL(m_sceneUpdater, handleSceneDataLinkRequest(MasterSceneId1, DataSlotId{2}, RefSceneId12, DataSlotId{1}));
        m_logic.update();

        // re-reference by new master
        SceneAllocateHelper otherMaster{ m_scenes.getScene(MasterSceneId2) };
        otherMaster.allocateSceneReference(RefSceneId11);

        // ref scene is now under new master which did not request any state for it yet, i.e. default is available
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        m_logic.update();
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // expect nothing
        m_logic.update();
    }
}
