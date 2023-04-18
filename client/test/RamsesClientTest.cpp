//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "ramses-client-api/EffectDescription.h"
#include "ramses-utils.h"

#include "RamsesClientImpl.h"
#include "SceneConfigImpl.h"
#include "Utils/File.h"
#include "RamsesObjectTestTypes.h"
#include "EffectImpl.h"
#include "ClientTestUtils.h"
#include "ClientApplicationLogic.h"
#include "SceneAPI/SceneId.h"
#include "Collections/String.h"
#include "ClientEventHandlerMock.h"

#include "SceneReferencing/SceneReferenceEvent.h"

namespace ramses
{
    using namespace testing;

    class ALocalRamsesClient : public LocalTestClient, public Test
    {
    public:
        ALocalRamsesClient()
        {
            effectDescriptionEmpty.setVertexShader("void main(void) {gl_Position=vec4(0);}");
            effectDescriptionEmpty.setFragmentShader("void main(void) {gl_FragColor=vec4(0);}");
        }

    protected:
        ramses::EffectDescription effectDescriptionEmpty;
    };

    class ARamsesClient : public Test
    {
    public:
        ARamsesClient()
            : m_client(*m_framework.createClient("client"))
        {
        }

    protected:
        RamsesFramework m_framework;
        RamsesClient& m_client;
    };

    TEST_F(ARamsesClient, canBeValidated)
    {
        EXPECT_EQ(StatusOK, m_client.validate());
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidRenderPass)
    {
        ramses::Scene* scene = m_client.createScene(sceneId_t(1), ramses::SceneConfig(), "");
        ASSERT_TRUE(nullptr != scene);

        RenderPass* passWithoutCamera = scene->createRenderPass();
        ASSERT_TRUE(nullptr != passWithoutCamera);

        EXPECT_NE(StatusOK, scene->validate());
        EXPECT_NE(StatusOK, m_client.validate());
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidCamera)
    {
        ramses::Scene* scene = m_client.createScene(sceneId_t(1), ramses::SceneConfig(), "");
        ASSERT_TRUE(nullptr != scene);

        Camera* cameraWithoutValidValues = scene->createPerspectiveCamera();
        ASSERT_TRUE(nullptr != cameraWithoutValidValues);

        EXPECT_NE(StatusOK, scene->validate());
        EXPECT_NE(StatusOK, m_client.validate());
    }

    TEST_F(ARamsesClient, noEventHandlerCallbacksIfNoEvents)
    {
        StrictMock<ClientEventHandlerMock> eventHandlerMock;
        m_client.dispatchEvents(eventHandlerMock);
    }

    // Not really useful and behavior is not defined, but should not crash at least
    TEST_F(ARamsesClient, canLiveParallelToAnotherClientUsingTheSameFramework)
    {
        RamsesClient& secondClient(*m_framework.createClient("client"));
        (void)secondClient;  //unused on purpose

        m_framework.connect();
        EXPECT_TRUE(m_framework.isConnected());
    }

    TEST_F(ALocalRamsesClient, requestNonexistantStatusMessage)
    {
        const char* msg = client.getStatusMessage(0xFFFFFFFF);
        EXPECT_TRUE(nullptr != msg);
    }

    TEST(RamsesClient, canCreateClientWithNULLNameAndCmdLineArguments)
    {
        ramses::RamsesFramework framework{ RamsesFrameworkConfig{} };
        EXPECT_NE(framework.createClient(nullptr), nullptr);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST(RamsesClient, canCreateClientWithoutCmdLineArguments)
    {
        ramses::RamsesFramework framework;
        EXPECT_NE(framework.createClient(nullptr), nullptr);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST(RamsesClient, createClientFailsWhenConnected)
    {
        ramses::RamsesFramework framework;
        EXPECT_EQ(ramses::StatusOK, framework.connect());
        EXPECT_EQ(framework.createClient(nullptr), nullptr);
    }

    TEST(RamsesClient, destroyClientFailsWhenConnected)
    {
        ramses::RamsesFramework framework;
        auto* client = framework.createClient(nullptr);
        ASSERT_NE(client, nullptr);
        EXPECT_EQ(ramses::StatusOK, framework.connect());
        EXPECT_NE(ramses::StatusOK, framework.destroyClient(*client));
        EXPECT_EQ(ramses::StatusOK, framework.disconnect());
        EXPECT_EQ(ramses::StatusOK, framework.destroyClient(*client));
    }

    TEST_F(ALocalRamsesClient, createsSceneWithGivenId)
    {
        const sceneId_t sceneId(33u);
        ramses::Scene* scene = client.createScene(sceneId);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_EQ(sceneId, scene->getSceneId());
    }

    TEST_F(ALocalRamsesClient, getsSceneWithGivenId)
    {
        const sceneId_t sceneId(33u);
        const sceneId_t sceneId2(44u);
        ramses::Scene* scene = client.createScene(sceneId);
        ramses::Scene* scene2 = client.createScene(sceneId2);
        ASSERT_TRUE(scene != nullptr);

        EXPECT_EQ(scene2, client.getScene(sceneId2));
        EXPECT_EQ(scene, client.getScene(sceneId));

        RamsesClient const& clientRef(client);
        EXPECT_EQ(scene, clientRef.getScene(sceneId));
        EXPECT_EQ(scene2, clientRef.getScene(sceneId2));
    }

    TEST_F(ALocalRamsesClient, doesNotGetSceneIfNoSceneOrWrongId)
    {
        const sceneId_t sceneId(33u);
        const sceneId_t sceneId2(44u);
        EXPECT_FALSE(client.getScene(sceneId));
        EXPECT_FALSE(client.getScene(sceneId2));
        ramses::Scene* scene = client.createScene(sceneId);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_FALSE(client.getScene(sceneId2));
    }

    TEST_F(ALocalRamsesClient, returnsNullptrWhenGettingDestroyedScene)
    {
        const sceneId_t sceneId(33u);
        ramses::Scene* scene = client.createScene(sceneId);
        ASSERT_TRUE(scene);
        EXPECT_EQ(client.destroy(*scene), StatusOK);
        EXPECT_FALSE(client.getScene(sceneId));
    }

    TEST_F(ALocalRamsesClient, createdSceneHasClientReference)
    {
        ramses::Scene* scene = client.createScene(sceneId_t(1));
        ASSERT_TRUE(scene != nullptr);
        EXPECT_EQ(&client, &scene->getRamsesClient());
    }

    TEST_F(ALocalRamsesClient, createsSceneFailsWithInvalidId)
    {
        EXPECT_EQ(nullptr, client.createScene(sceneId_t{}));
    }

    TEST_F(ALocalRamsesClient, simpleSceneGetsDestroyedProperlyWithoutExplicitDestroyCall)
    {
        ramses::Scene* scene = client.createScene(sceneId_t(1u));
        EXPECT_TRUE(scene != nullptr);
        ramses::Node* node = scene->createNode("node");
        EXPECT_TRUE(node != nullptr);
    }

    TEST_F(ALocalRamsesClient, requestValidStatusMessage)
    {
        LocalTestClient otherClient;
        Scene& scene = otherClient.createObject<Scene>();

        ramses::status_t status = client.destroy(scene);
        EXPECT_NE(ramses::StatusOK, status);
        const char* msg = client.getStatusMessage(status);
        EXPECT_TRUE(nullptr != msg);
    }

    TEST_F(ALocalRamsesClient, isUnpublishedOnDestroy)
    {
        const sceneId_t sceneId(45u);
        ramses::Scene* scene = client.createScene(sceneId);

        using ramses_internal::SceneInfoVector;
        using ramses_internal::SceneInfo;
        ramses_internal::SceneId internalSceneId(sceneId.getValue());

        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(SceneInfo(internalSceneId, ramses_internal::String(scene->getName())), _));
        EXPECT_CALL(sceneActionsCollector, handleInitializeScene(_, _));
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses_internal::SceneId(sceneId.getValue()), _, _));
        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(internalSceneId, _));

        scene->publish(ramses::EScenePublicationMode_LocalOnly);
        scene->flush();
        EXPECT_TRUE(client.impl.getClientApplication().isScenePublished(internalSceneId));

        client.destroy(*scene);
        EXPECT_FALSE(client.impl.getClientApplication().isScenePublished(internalSceneId));
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfThereIsNoSceneReference)
    {
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), nullptr);
        client.createScene(sceneId_t{ 123 });
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfWrongReferencedSceneIdIsProvided)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        scene->createSceneReference(sceneId_t{ 456 });

        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 1 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfWrongMasterSceneIdIsProvided)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        scene->createSceneReference(sceneId_t{ 456 });
        auto scene2 = client.createScene(sceneId_t{ 1234 });
        scene2->createSceneReference(sceneId_t{ 4567 });

        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 1 }, sceneId_t{ 456 }), nullptr);
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 1234 }, sceneId_t{ 456 }), nullptr);
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 4567 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsSceneReferenceOnFindSceneReference)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        auto scene2 = client.createScene(sceneId_t{ 1234 });
        auto sr = scene->createSceneReference(sceneId_t{ 456 });
        auto sr2 = scene->createSceneReference(sceneId_t{ 12345 });
        auto sr3 = scene2->createSceneReference(sceneId_t{ 12345 });

        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), sr);
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 123 }, sceneId_t{ 12345 }), sr2);
        EXPECT_EQ(client.impl.findSceneReference(sceneId_t{ 1234 }, sceneId_t{ 12345 }), sr3);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForSceneStateChangedEvent)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses_internal::SceneReferenceEventType::SceneStateChanged;
        event.sceneState = ramses_internal::RendererSceneState::Rendered;

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceStateChanged(_, RendererSceneState::Rendered)).WillOnce([sr](SceneReference& ref, RendererSceneState)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForSceneFlushedEvent)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses_internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses_internal::SceneVersionTag{ 567 };

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 567 })).WillOnce([sr](SceneReference& ref, sceneVersionTag_t)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForDataLinkedEvent)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.type = ramses_internal::SceneReferenceEventType::DataLinked;
        event.providerScene = masterScene;
        event.consumerScene = reffedScene;
        event.dataProvider = ramses_internal::DataSlotId{ 123 };
        event.dataConsumer = ramses_internal::DataSlotId{ 987 };
        event.status = false;

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, dataLinked(sceneId_t{ masterScene.getValue() }, dataProviderId_t{ 123 }, sceneId_t{ reffedScene.getValue() }, dataConsumerId_t{ 987 }, false));
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForDataUnlinkedEvent)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.type = ramses_internal::SceneReferenceEventType::DataUnlinked;
        event.consumerScene = reffedScene;
        event.dataConsumer = ramses_internal::DataSlotId{ 987 };
        event.status = true;

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, dataUnlinked(sceneId_t{ reffedScene.getValue() }, dataConsumerId_t{ 987 }, true));
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsNotificationForEachEvent)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses_internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses_internal::SceneVersionTag{ 567 };
        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});
        event.tag = ramses_internal::SceneVersionTag{ 568 };
        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});
        event.tag = ramses_internal::SceneVersionTag{ 569 };
        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 567 })).WillOnce([sr](SceneReference& ref, sceneVersionTag_t)
            {
                EXPECT_EQ(&ref, sr);
            });
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 568 })).WillOnce([sr](SceneReference& ref, sceneVersionTag_t)
            {
                EXPECT_EQ(&ref, sr);
            });
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 569 })).WillOnce([sr](SceneReference& ref, sceneVersionTag_t)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, doesNotCallAnyNotificationIfSceneReferenceDoesNotExistForNonDataLinkEvents)
    {
        constexpr auto masterScene = ramses_internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses_internal::SceneId{ 456 };

        client.createScene(sceneId_t{ masterScene.getValue() });

        ramses_internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses_internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses_internal::SceneVersionTag{ 567 };

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        event.referencedScene = reffedScene;
        event.type = ramses_internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses_internal::SceneVersionTag{ 567 };

        client.impl.getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        client.dispatchEvents(handler);
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateAClient)
    {
        ramses::RamsesFramework fw;

        auto client = fw.createClient("client");
        EXPECT_NE(client, nullptr);
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateMultipleClients)
    {
        ramses::RamsesFramework fw;

        auto client1 = fw.createClient("first client");
        auto client2 = fw.createClient("second client");
        EXPECT_NE(nullptr, client1);
        EXPECT_NE(nullptr, client2);
    }

    TEST(ARamsesFrameworkImplInAClientLib, acceptsLocallyCreatedClientsForDestruction)
    {
        ramses::RamsesFramework fw;

        auto client1 = fw.createClient("first client");
        auto client2 = fw.createClient("second client");
        EXPECT_EQ(StatusOK, fw.destroyClient(*client1));
        EXPECT_EQ(StatusOK, fw.destroyClient(*client2));
    }

    TEST(ARamsesFrameworkImplInAClientLib, doesNotAcceptForeignCreatedClientsForDestruction)
    {
        ramses::RamsesFramework fw1;
        ramses::RamsesFramework fw2;

        auto client1 = fw1.createClient("first client");
        auto client2 = fw2.createClient("second client");
        EXPECT_NE(StatusOK, fw2.destroyClient(*client1));
        EXPECT_NE(StatusOK, fw1.destroyClient(*client2));
    }

    TEST(ARamsesFrameworkImplInAClientLib, doesNotAcceptSameClientTwiceForDestruction)
    {
        ramses::RamsesFramework fw;

        auto client = fw.createClient("client");
        EXPECT_EQ(StatusOK, fw.destroyClient(*client));
        EXPECT_NE(StatusOK, fw.destroyClient(*client));
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateDestroyAndRecreateAClient)
    {
        ramses::RamsesFramework fw;

        auto client = fw.createClient("client");
        EXPECT_NE(nullptr, client);
        EXPECT_EQ(fw.destroyClient(*client), StatusOK);
        client = fw.createClient("client");
        EXPECT_NE(nullptr, client);
    }
}
