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
#include "RendererSceneControlMock.h"
#include "RendererSceneControlLogicMock.h"
#include "RendererSceneEventSenderMock.h"
#include "RendererEventCollector.h"
#include "SceneAllocateHelper.h"

using namespace testing;

namespace ramses_internal
{
    class ASceneReferenceLogic : public Test
    {
    public:
        ASceneReferenceLogic()
            : m_scenes(m_eventCollector)
            , m_logic(m_scenes, m_sceneLogic, m_sceneControl, m_eventSender)
        {
        }

        virtual void SetUp() override
        {
            SceneAllocateHelper masterScene1{ m_scenes.createScene({ MasterSceneId1 }) };
            masterScene1.allocateSceneReference(RefSceneId11, RefSceneHandle11);
            masterScene1.allocateSceneReference(RefSceneId12, RefSceneHandle12);
            SceneAllocateHelper masterScene2{ m_scenes.createScene({ MasterSceneId2 }) };
            masterScene2.allocateSceneReference(RefSceneId21, RefSceneHandle21);
            masterScene2.allocateSceneReference(RefSceneId22, RefSceneHandle22);

            m_scenes.createScene({ RefSceneId11 });
            m_scenes.createScene({ RefSceneId12 });
            m_scenes.createScene({ RefSceneId21 });
            m_scenes.createScene({ RefSceneId22 });

            // ignore this unless overriden by test
            EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _, _)).Times(AnyNumber());
        }

    protected:
        void updateLogicAndVerifyExpectations()
        {
            m_logic.update();
            Mock::VerifyAndClearExpectations(&m_sceneLogic);
            Mock::VerifyAndClearExpectations(&m_sceneControl);

            // ignore this unless overriden by test
            EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _, _)).Times(AnyNumber());
        }

        RendererEventCollector m_eventCollector;
        RendererScenes m_scenes;
        StrictMock<RendererSceneControlLogicMock> m_sceneLogic;
        StrictMock<RendererSceneControlMock> m_sceneControl;
        StrictMock<RendererSceneEventSenderMock> m_eventSender;
        SceneReferenceLogic m_logic;

        static constexpr SceneId MasterSceneId1{ 123 };
        static constexpr SceneId MasterSceneId2{ 124 };
        static constexpr SceneId RefSceneId11{ 125 };
        static constexpr SceneId RefSceneId12{ 126 };
        static constexpr SceneId RefSceneId21{ 127 };
        static constexpr SceneId RefSceneId22{ 128 };
        static constexpr SceneReferenceHandle RefSceneHandle11{ 13 };
        static constexpr SceneReferenceHandle RefSceneHandle12{ 14 };
        static constexpr SceneReferenceHandle RefSceneHandle21{ 15 };
        static constexpr SceneReferenceHandle RefSceneHandle22{ 16 };
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
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId21, _, _, _, _));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId22, _, _, _, _));
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());
    }

    TEST_F(ASceneReferenceLogic, executesActionForMasterScene)
    {
        static constexpr SceneReferenceAction linkAction1{ SceneReferenceActionType::LinkData, {}, DataSlotId{1}, RefSceneHandle11, DataSlotId{2} };
        m_logic.addActions(MasterSceneId1, { linkAction1 });

        EXPECT_CALL(m_sceneControl, handleSceneDataLinkRequest(RefSceneId11, linkAction1.providerId, MasterSceneId1, linkAction1.consumerId));
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

        EXPECT_CALL(m_sceneControl, handleSceneDataLinkRequest(RefSceneId11, linkAction1.providerId, MasterSceneId1, linkAction1.consumerId));
        EXPECT_CALL(m_sceneControl, handleSceneDataLinkRequest(MasterSceneId1, linkAction2.providerId, RefSceneId12, linkAction2.consumerId));
        EXPECT_CALL(m_sceneControl, handleDataUnlinkRequest(MasterSceneId1, unlinkAction1.consumerId));
        EXPECT_CALL(m_sceneControl, handleDataUnlinkRequest(MasterSceneId2, unlinkAction1.consumerId));
        EXPECT_CALL(m_sceneControl, handleDataUnlinkRequest(RefSceneId22, unlinkAction2.consumerId));
        updateLogicAndVerifyExpectations();

        // actions cleared, no more expectations
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, requestsStateChangeForReferencedScene)
    {
        // simulate master scene Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Available);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, delaysRequestStateChangeForReferencedSceneUntilMasterSceneAtLeastAtSameStateLevel)
    {
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Available);
        // no expecation, master scene is unavailable
        updateLogicAndVerifyExpectations();

        // simulate master scene Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, reactsToMasterSceneStateChangeSoThatReferencedScenesNeverHaveHigherStateThanMaster_fromAvailableUp)
    {
        // simulate master scene Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Available; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Available; }));

        // simulate master scene state Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // ref scenes now can advance to Ready as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state to Rendered
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
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
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Rendered;
        }));

        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Rendered);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Ready);

        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Rendered));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Rendered; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state dropped to Ready
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Ready;
        }));

        // previously rendered ref scene dropped to Ready as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Ready));
        updateLogicAndVerifyExpectations();

        // next query ref scenes will report their requested states
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Ready; }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&) { targetState = RendererSceneState::Ready; }));

        // simulate master scene state dropped to Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        // both ref scenes drop to Available as well
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referencedScenesInheritMappingInformationFromMasterScene)
    {
        constexpr DisplayHandle display{ 2 };
        constexpr OffscreenBufferHandle ob{ 3 };

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
        {
            display_ = display;
            ob_ = ob;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId11, display));
        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId12, display));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob, -13)); // requested render order is relative to master
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob, -13)); // requested render order is relative to master
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, referencedScenesGetUpdatedMappingInformationFromMasterScene)
    {
        constexpr DisplayHandle display1{ 2 };
        constexpr DisplayHandle display2{ 3 };
        constexpr OffscreenBufferHandle ob1{ 4 };
        constexpr OffscreenBufferHandle ob2{ 5 };

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
        {
            display_ = display1;
            ob_ = ob1;
            renderOrder = -13;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId11, display1));
        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId12, display1));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13)); // requested render order is relative to master
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, -13)); // requested render order is relative to master
        updateLogicAndVerifyExpectations();

        // simulate master scene has new mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillOnce(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
        {
            display_ = display2;
            ob_ = ob2;
            renderOrder = -3;
        }));

        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId11, display2));
        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId12, display2));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob2, -3)); // requested render order is relative to master
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob2, -3)); // requested render order is relative to master
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, setsRenderOrderForReferencedScene)
    {
        constexpr DisplayHandle display1{ 2 };
        constexpr OffscreenBufferHandle ob1{ 4 };

        // first set mapping information for master scene which will trigger setting mapping info on its references
        // scene render order can be set only after mapping info available
        {
            // simulate master scene has mapping set
            EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
            {
                display_ = display1;
                ob_ = ob1;
                renderOrder = -13;
            }));

            EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId11, display1));
            EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId12, display1));
            EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13)); // requested render order is relative to master
            EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, -13)); // requested render order is relative to master
            m_logic.update();

            // next time a ref scene is queried for mapping info use the last requested values
            EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId11, _, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
            {
                display_ = display1;
                ob_ = ob1;
                renderOrder = -13;
            }));
            EXPECT_CALL(m_sceneLogic, getSceneInfo(RefSceneId12, _, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
            {
                display_ = display1;
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
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
        {
            display_ = display1;
            ob_ = ob1;
            renderOrder = 123;
        }));

        // will update both referenced scenes' render order
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, 123 + 1234));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, 123 + 0));
        m_logic.update();
    }

    TEST_F(ASceneReferenceLogic, delaysSettingRenderOrderForReferencedSceneUntilMasterSceneMappingIsKnown)
    {
        constexpr DisplayHandle display1{ 2 };
        constexpr OffscreenBufferHandle ob1{ 4 };

        // set render order
        m_scenes.getScene(MasterSceneId1).setSceneReferenceRenderOrder(RefSceneHandle11, -666);
        // no expectations as mapping is not known yet
        updateLogicAndVerifyExpectations();

        // simulate master scene has mapping set
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillRepeatedly(Invoke([&](auto, auto&, auto& display_, auto& ob_, auto& renderOrder)
        {
            display_ = display1;
            ob_ = ob1;
            renderOrder = -13;
        }));

        // now will set render order together with mapping
        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId11, display1));
        EXPECT_CALL(m_sceneLogic, setSceneMapping(RefSceneId12, display1));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId11, ob1, -13 - 666));
        EXPECT_CALL(m_sceneLogic, setSceneDisplayBufferAssignment(RefSceneId12, ob1, -13)); // requested render order is relative to master
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, requestsUnavailableStateForAllReferencedScenesOfDestroyedMasterScene)
    {
        // simulate master scenes Available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId1, _, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));
        EXPECT_CALL(m_sceneLogic, getSceneInfo(MasterSceneId2, _, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        // request ref scenes available
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle11, RendererSceneState::Available);
        m_scenes.getScene(MasterSceneId1).requestSceneReferenceState(RefSceneHandle12, RendererSceneState::Available);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(RefSceneHandle21, RendererSceneState::Available);
        m_scenes.getScene(MasterSceneId2).requestSceneReferenceState(RefSceneHandle22, RendererSceneState::Available);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Available));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Available));
        updateLogicAndVerifyExpectations();

        // now report every scene as requested available
        EXPECT_CALL(m_sceneLogic, getSceneInfo(_, _, _, _, _)).WillRepeatedly(Invoke([](auto, auto& targetState, auto&, auto&, auto&)
        {
            targetState = RendererSceneState::Available;
        }));

        // destroy one master
        m_scenes.destroyScene(MasterSceneId2);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Unavailable));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Unavailable));
        updateLogicAndVerifyExpectations();

        // destroy another master
        m_scenes.destroyScene(MasterSceneId1);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId11, RendererSceneState::Unavailable));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId12, RendererSceneState::Unavailable));
        updateLogicAndVerifyExpectations();
    }

    TEST_F(ASceneReferenceLogic, doesNotSendNorExtractPublishEventOfReferencedScene)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEvent event{ ERendererEventType_ScenePublished };
        event.sceneId = RefSceneId12;
        RendererEventVector events{ event };

        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_ScenePublished, events[0].eventType);
        EXPECT_EQ(RefSceneId12, events[0].sceneId);
    }

    TEST_F(ASceneReferenceLogic, doesNotSendNorExtractEventsOfUninterestingType)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events{ { ERendererEventType_DisplayCreated }, { ERendererEventType_WindowKeyEvent }, { ERendererEventType_ReadPixelsFromFramebuffer } };
        RendererEventVector eventsOriginal = events;

        m_logic.extractAndSendSceneReferenceEvents(events);
        ASSERT_EQ(eventsOriginal.size(), events.size());
        for (size_t i = 0u; i < events.size(); ++i)
            EXPECT_EQ(eventsOriginal[i].eventType, events[i].eventType);
    }

    TEST_F(ASceneReferenceLogic, extractsButDoesNotSendSceneRefEventsOfImplicitOrIrrelevantTypes)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events
        {
            { ERendererEventType_SceneAssignedToDisplayBuffer }, { ERendererEventType_SceneAssignedToDisplayBufferFailed }, { ERendererEventType_SceneDataSlotProviderCreated },
            { ERendererEventType_DisplayCreated }, // will be kept
            { ERendererEventType_SceneDataSlotProviderDestroyed }, { ERendererEventType_SceneDataSlotConsumerCreated }, { ERendererEventType_SceneDataSlotConsumerDestroyed }
        };
        // all of them 'belong' to ref scene
        for (auto& evt : events)
            evt.sceneId = RefSceneId12;
        RendererEventVector eventsOriginal = events;

        m_logic.extractAndSendSceneReferenceEvents(events);

        // display creation event was the only one kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_DisplayCreated, events[0].eventType);
    }

    TEST_F(ASceneReferenceLogic, extractsButDoesNotSendSceneRefEventsOfUnusedDataLinkingTypes)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events
        {
            { ERendererEventType_SceneDataBufferLinked },
            { ERendererEventType_SceneDataBufferLinkFailed }, { ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange },
            { ERendererEventType_DisplayCreated }, // will be kept
        };
        // all of them have ref scene as consumer scene
        for (auto& evt : events)
            evt.consumerSceneId = RefSceneId12;
        RendererEventVector eventsOriginal = events;

        m_logic.extractAndSendSceneReferenceEvents(events);

        // display creation event was the only one kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_DisplayCreated, events[0].eventType);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneStateChange)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType_SceneStateChanged };
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
        EXPECT_EQ(ERendererEventType_SceneStateChanged, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(RendererSceneState::Ready, events[0].state);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneStateChange_masterSceneAlreadyDestroyed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        m_scenes.destroyScene(MasterSceneId2);
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId21, RendererSceneState::Unavailable));
        EXPECT_CALL(m_sceneLogic, setSceneState(RefSceneId22, RendererSceneState::Unavailable));
        updateLogicAndVerifyExpectations();

        RendererEventVector events;
        RendererEvent event{ ERendererEventType_SceneStateChanged };
        event.sceneId = RefSceneId21;
        event.state = RendererSceneState::Ready;
        events.push_back(event);

        // ref scene event will be extracted and sent
        EXPECT_CALL(m_eventSender, sendSceneStateChanged(MasterSceneId2, RefSceneId21, RendererSceneState::Ready));
        m_logic.extractAndSendSceneReferenceEvents(events);
        EXPECT_TRUE(events.empty());
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_SceneFlushed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        constexpr SceneVersionTag version{ 123 };
        RendererEventVector events;
        RendererEvent event{ ERendererEventType_SceneFlushed };
        event.sceneId = RefSceneId21;
        event.sceneVersionTag = version;
        events.push_back(event);

        // same event but for not referenced scene
        event.sceneId = MasterSceneId1;
        events.push_back(event);

        // ref scene event will be extracted and sent
        EXPECT_CALL(m_eventSender, sendSceneFlushed(MasterSceneId2, RefSceneId21, version));
        m_logic.extractAndSendSceneReferenceEvents(events);

        // non-ref scene event will is kept
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(ERendererEventType_SceneFlushed, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].sceneId);
        EXPECT_EQ(version, events[0].sceneVersionTag);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataLinked)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType_SceneDataLinked };
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
        EXPECT_EQ(ERendererEventType_SceneDataLinked, events[0].eventType);
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
        RendererEvent event{ ERendererEventType_SceneDataLinkFailed };
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
        EXPECT_EQ(ERendererEventType_SceneDataLinkFailed, events[0].eventType);
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
        RendererEvent event{ ERendererEventType_SceneDataUnlinked };
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
        EXPECT_EQ(ERendererEventType_SceneDataUnlinked, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, extractsAndSendsEventOfReferencedScene_DataUnlinkFailed)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events;
        RendererEvent event{ ERendererEventType_SceneDataUnlinkFailed };
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
        EXPECT_EQ(ERendererEventType_SceneDataUnlinkFailed, events[0].eventType);
        EXPECT_EQ(MasterSceneId1, events[0].providerSceneId);
        EXPECT_EQ(MasterSceneId2, events[0].consumerSceneId);
        EXPECT_EQ(DataSlotId{ 8 }, events[0].consumerdataId);
    }

    TEST_F(ASceneReferenceLogic, transformsButDoesNotSendSceneRefEventsOfToBeReportedAsMasterSceneEvents)
    {
        updateLogicAndVerifyExpectations();
        EXPECT_TRUE(m_logic.hasAnyReferencedScenes());

        RendererEventVector events
        {
            { ERendererEventType_SceneExpired }, { ERendererEventType_SceneRecoveredFromExpiration }
        };
        events[0].sceneId = RefSceneId22;
        events[1].sceneId = RefSceneId11;

        m_logic.extractAndSendSceneReferenceEvents(events);

        // events are kept but changed ownership to their corresponding master scene
        ASSERT_EQ(2u, events.size());
        EXPECT_EQ(ERendererEventType_SceneExpired, events[0].eventType);
        EXPECT_EQ(MasterSceneId2, events[0].sceneId);
        EXPECT_EQ(ERendererEventType_SceneRecoveredFromExpiration, events[1].eventType);
        EXPECT_EQ(MasterSceneId1, events[1].sceneId);
    }
}
