//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "ClientTestUtils.h"
#include "ramses/client/Node.h"
#include "internal/SceneGraph/SceneAPI/IScene.h"
#include "internal/SceneGraph/Scene/SceneActionCollection.h"


namespace ramses::internal
{
    using namespace testing;
    class ADistributedScene : public LocalTestClientWithScene, public ::testing::TestWithParam<EScenePublicationMode>
    {
    public:
        ADistributedScene() : LocalTestClientWithScene(GetParam())
        {
            framework.connect();
        }

        void publishScene()
        {
            const ramses::internal::IScene& iscene = m_scene.impl().getIScene();
            ramses::internal::SceneInfo info(iscene.getSceneId(), iscene.getName());
            EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(info, _));
            EXPECT_CALL(sceneActionsCollector, handleInitializeScene(info, _));
            EXPECT_TRUE(m_scene.publish(GetParam()));
        }

        void unpublishScene()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(ramses::internal::SceneId(m_scene.impl().getSceneId().getValue()), _));
            EXPECT_TRUE(m_scene.unpublish());
        }

        void doSceneOperations()
        {
            // do random m_scene stuff
            Node* node = m_scene.createNode("node");
            Node* nodeTrans = m_scene.createNode("nodetrans");
            nodeTrans->setParent(*node);
        }

        static void ExpectActionListsEqual(const ramses::internal::SceneActionCollection& list1, const ramses::internal::SceneActionCollection& list2)
        {
            EXPECT_EQ(list1.numberOfActions(), list2.numberOfActions());
            for (uint32_t i = 0u; i < list1.numberOfActions(); ++i)
            {
                EXPECT_EQ(list1[i].type(), list2[i].type());
            }
        }

        void expectSceneOperationsSent()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(m_scene.impl().getSceneId().getValue()), _, _));
        }

        void expectSceneUnpublication()
        {
            EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(ramses::internal::SceneId(m_scene.impl().getSceneId().getValue()), _));
        }

    protected:
    };

    INSTANTIATE_TEST_SUITE_P(
        ADistributedScene_Suite,
        ADistributedScene,
        ::testing::Values(EScenePublicationMode::LocalOnly, EScenePublicationMode::LocalAndRemote));

    TEST_P(ADistributedScene, flushProducesSingleActionList)
    {
        publishScene();
        expectSceneOperationsSent();
        EXPECT_TRUE(m_scene.flush());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        expectSceneOperationsSent();
        EXPECT_TRUE(m_scene.flush());
        ramses::internal::SceneActionCollection actionsBeforeFirstFlush(sceneActionsCollector.getCopyOfCollectedActions());
        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        expectSceneOperationsSent();
        EXPECT_TRUE(m_scene.flush());
        ramses::internal::SceneActionCollection actionsBeforeSecondFlush(sceneActionsCollector.getCopyOfCollectedActions());
        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        ExpectActionListsEqual(actionsBeforeFirstFlush, actionsBeforeSecondFlush);
        expectSceneUnpublication();
    }

    TEST_P(ADistributedScene, publishCausesSceneToBeSentToImplicitSubscriber)
    {
        expectSceneOperationsSent();
        m_scene.flush();
        sceneActionsCollector.resetCollecting();
        doSceneOperations();
        publishScene();

        if (GetParam() == EScenePublicationMode::LocalOnly) // in local only case scene has to be flush to send anything
            m_scene.flush();

        EXPECT_EQ(1u, sceneActionsCollector.getNumReceivedActionLists());
        expectSceneUnpublication();
    }

    TEST_P(ADistributedScene, unpublishingSceneResetsPendingActions)
    {
        publishScene();
        expectSceneOperationsSent();
        EXPECT_TRUE(m_scene.flush());
        sceneActionsCollector.resetCollecting();

        doSceneOperations();
        unpublishScene();

        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        sceneActionsCollector.resetCollecting();

        // no new messages/calls, everything has been already flushed
        EXPECT_TRUE(m_scene.flush());
        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        EXPECT_EQ(0u, sceneActionsCollector.getNumberOfActions());
    }

    TEST_P(ADistributedScene, destroyingSceneCausesUnpublish)
    {
        const ramses::internal::SceneId sceneId(33u);
        ramses::Scene* otherScene = client.createScene(sceneId_t(sceneId.getValue()));
        ASSERT_TRUE(otherScene != nullptr);
        const ramses::internal::IScene& otherIScene = otherScene->impl().getIScene();

        ramses::internal::SceneInfo sceneInfo(sceneId, otherIScene.getName());
        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(sceneInfo, _));
        EXPECT_TRUE(otherScene->publish());

        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(sceneId, _));
        client.destroy(*otherScene);
    }

    TEST_P(ADistributedScene, confidence_emptyFlushDoesNotCollectAnySceneActions)
    {
        // First do immediate mode, no commits
        publishScene();
        doSceneOperations();
        expectSceneOperationsSent();
        m_scene.flush();
        sceneActionsCollector.resetCollecting();
        m_scene.flush();
        EXPECT_EQ(0u, sceneActionsCollector.getNumReceivedActionLists());
        expectSceneUnpublication();
    }
}
