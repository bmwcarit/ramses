//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "ramses/client/EffectDescription.h"
#include "ramses/client/ramses-utils.h"

#include "impl/RamsesClientImpl.h"
#include "impl/SceneConfigImpl.h"
#include "internal/Core/Utils/File.h"
#include "RamsesObjectTestTypes.h"
#include "impl/EffectImpl.h"
#include "ClientTestUtils.h"
#include "internal/ClientApplicationLogic.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "ClientEventHandlerMock.h"

#include "internal/SceneReferencing/SceneReferenceEvent.h"

namespace ramses::internal
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
        EffectDescription effectDescriptionEmpty;
    };

    class ARamsesClient : public Test
    {
    public:
        ARamsesClient()
            : m_client(*m_framework.createClient("client"))
        {
        }

    protected:
        RamsesFramework m_framework{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        RamsesClient& m_client;
    };

    TEST_F(ARamsesClient, canBeValidated)
    {
        ValidationReport report;
        m_client.validate(report);
        EXPECT_FALSE(report.hasIssue());
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidRenderPass)
    {
        ramses::Scene* scene = m_client.createScene(sceneId_t(1), "");
        ASSERT_TRUE(nullptr != scene);

        ramses::RenderPass* passWithoutCamera = scene->createRenderPass();
        ASSERT_TRUE(nullptr != passWithoutCamera);

        {
            ValidationReport report;
            scene->validate(report);
            EXPECT_TRUE(report.hasIssue());
        }
        {
            ValidationReport report;
            m_client.validate(report);
            EXPECT_TRUE(report.hasIssue());
        }
    }

    TEST_F(ARamsesClient, failsValidationWhenContainsSceneWithInvalidCamera)
    {
        ramses::Scene* scene = m_client.createScene(sceneId_t(1), "");
        ASSERT_TRUE(nullptr != scene);

        ramses::Camera* cameraWithoutValidValues = scene->createPerspectiveCamera();
        ASSERT_TRUE(nullptr != cameraWithoutValidValues);

        {
            ValidationReport report;
            scene->validate(report);
            EXPECT_TRUE(report.hasError());
        }
        {
            ValidationReport report;
            m_client.validate(report);
            EXPECT_TRUE(report.hasError());
        }
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

    TEST(RamsesClient, canCreateClientWithNULLNameAndCmdLineArguments)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework framework{config};
        EXPECT_NE(framework.createClient({}), nullptr);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST(RamsesClient, canCreateClientWithoutCmdLineArguments)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework framework{config};
        EXPECT_NE(framework.createClient({}), nullptr);
        EXPECT_FALSE(framework.isConnected());
    }

    TEST(RamsesClient, createClientFailsWhenConnected)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework framework{config};
        EXPECT_TRUE(framework.connect());
        EXPECT_EQ(framework.createClient({}), nullptr);
    }

    TEST(RamsesClient, destroyClientFailsWhenConnected)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework framework{config};
        auto* client = framework.createClient({});
        ASSERT_NE(client, nullptr);
        EXPECT_TRUE(framework.connect());
        EXPECT_FALSE(framework.destroyClient(*client));
        EXPECT_TRUE(framework.disconnect());
        EXPECT_TRUE(framework.destroyClient(*client));
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
        EXPECT_TRUE(client.destroy(*scene));
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

    TEST_F(ALocalRamsesClient, isUnpublishedOnDestroy)
    {
        const sceneId_t sceneId(45u);
        ramses::Scene* scene = client.createScene(sceneId);

        using ramses::internal::SceneInfoVector;
        using ramses::internal::SceneInfo;
        ramses::internal::SceneId internalSceneId(sceneId.getValue());

        EXPECT_CALL(sceneActionsCollector, handleNewSceneAvailable(SceneInfo(internalSceneId, scene->getName()), _));
        EXPECT_CALL(sceneActionsCollector, handleInitializeScene(_, _));
        EXPECT_CALL(sceneActionsCollector, handleSceneUpdate_rvr(ramses::internal::SceneId(sceneId.getValue()), _, _));
        EXPECT_CALL(sceneActionsCollector, handleSceneBecameUnavailable(internalSceneId, _));

        scene->publish(EScenePublicationMode::LocalOnly);
        scene->flush();
        EXPECT_TRUE(client.impl().getClientApplication().isScenePublished(internalSceneId));

        client.destroy(*scene);
        EXPECT_FALSE(client.impl().getClientApplication().isScenePublished(internalSceneId));
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfThereIsNoSceneReference)
    {
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), nullptr);
        client.createScene(sceneId_t{ 123 });
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfWrongReferencedSceneIdIsProvided)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        scene->createSceneReference(sceneId_t{ 456 });

        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 1 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsNullptrOnFindSceneReferenceIfWrongMasterSceneIdIsProvided)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        scene->createSceneReference(sceneId_t{ 456 });
        auto scene2 = client.createScene(sceneId_t{ 1234 });
        scene2->createSceneReference(sceneId_t{ 4567 });

        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 1 }, sceneId_t{ 456 }), nullptr);
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 1234 }, sceneId_t{ 456 }), nullptr);
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 4567 }), nullptr);
    }

    TEST_F(ALocalRamsesClient, returnsSceneReferenceOnFindSceneReference)
    {
        auto scene = client.createScene(sceneId_t{ 123 });
        auto scene2 = client.createScene(sceneId_t{ 1234 });
        auto sr = scene->createSceneReference(sceneId_t{ 456 });
        auto sr2 = scene->createSceneReference(sceneId_t{ 12345 });
        auto sr3 = scene2->createSceneReference(sceneId_t{ 12345 });

        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 456 }), sr);
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 123 }, sceneId_t{ 12345 }), sr2);
        EXPECT_EQ(client.impl().findSceneReference(sceneId_t{ 1234 }, sceneId_t{ 12345 }), sr3);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForSceneStateChangedEvent)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses::internal::SceneReferenceEventType::SceneStateChanged;
        event.sceneState = RendererSceneState::Rendered;

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceStateChanged(_, RendererSceneState::Rendered)).WillOnce([sr](ramses::SceneReference& ref, RendererSceneState /*unused*/)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForSceneFlushedEvent)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses::internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses::internal::SceneVersionTag{ 567 };

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 567 })).WillOnce([sr](ramses::SceneReference& ref, sceneVersionTag_t /*version*/)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForDataLinkedEvent)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.type = ramses::internal::SceneReferenceEventType::DataLinked;
        event.providerScene = masterScene;
        event.consumerScene = reffedScene;
        event.dataProvider = ramses::internal::DataSlotId{ 123 };
        event.dataConsumer = ramses::internal::DataSlotId{ 987 };
        event.status = false;

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, dataLinked(sceneId_t{ masterScene.getValue() }, dataProviderId_t{ 123 }, sceneId_t{ reffedScene.getValue() }, dataConsumerId_t{ 987 }, false));
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsAppropriateNotificationForDataUnlinkedEvent)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.type = ramses::internal::SceneReferenceEventType::DataUnlinked;
        event.consumerScene = reffedScene;
        event.dataConsumer = ramses::internal::DataSlotId{ 987 };
        event.status = true;

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, dataUnlinked(sceneId_t{ reffedScene.getValue() }, dataConsumerId_t{ 987 }, true));
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, callsNotificationForEachEvent)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        auto scene = client.createScene(sceneId_t{ masterScene.getValue() });
        auto sr = scene->createSceneReference(sceneId_t{ reffedScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses::internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses::internal::SceneVersionTag{ 567 };
        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});
        event.tag = ramses::internal::SceneVersionTag{ 568 };
        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});
        event.tag = ramses::internal::SceneVersionTag{ 569 };
        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 567 })).WillOnce([sr](ramses::SceneReference& ref, sceneVersionTag_t /*version*/)
            {
                EXPECT_EQ(&ref, sr);
            });
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 568 })).WillOnce([sr](ramses::SceneReference& ref, sceneVersionTag_t /*version*/)
            {
                EXPECT_EQ(&ref, sr);
            });
        EXPECT_CALL(handler, sceneReferenceFlushed(_, sceneVersionTag_t{ 569 })).WillOnce([sr](ramses::SceneReference& ref, sceneVersionTag_t /*version*/)
            {
                EXPECT_EQ(&ref, sr);
            });
        client.dispatchEvents(handler);
    }

    TEST_F(ALocalRamsesClient, doesNotCallAnyNotificationIfSceneReferenceDoesNotExistForNonDataLinkEvents)
    {
        constexpr auto masterScene = ramses::internal::SceneId{ 123 };
        constexpr auto reffedScene = ramses::internal::SceneId{ 456 };

        client.createScene(sceneId_t{ masterScene.getValue() });

        ramses::internal::SceneReferenceEvent event(masterScene);
        event.referencedScene = reffedScene;
        event.type = ramses::internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses::internal::SceneVersionTag{ 567 };

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        event.referencedScene = reffedScene;
        event.type = ramses::internal::SceneReferenceEventType::SceneFlushed;
        event.tag = ramses::internal::SceneVersionTag{ 567 };

        client.impl().getClientApplication().handleSceneReferenceEvent(event, {});

        testing::StrictMock<ClientEventHandlerMock> handler;
        client.dispatchEvents(handler);
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateAClient)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw{config};

        auto client = fw.createClient("client");
        EXPECT_NE(client, nullptr);
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateMultipleClients)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw{config};

        auto client1 = fw.createClient("first client");
        auto client2 = fw.createClient("second client");
        EXPECT_NE(nullptr, client1);
        EXPECT_NE(nullptr, client2);
    }

    TEST(ARamsesFrameworkImplInAClientLib, acceptsLocallyCreatedClientsForDestruction)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw{config};

        auto client1 = fw.createClient("first client");
        auto client2 = fw.createClient("second client");
        EXPECT_TRUE(fw.destroyClient(*client1));
        EXPECT_TRUE(fw.destroyClient(*client2));
    }

    TEST(ARamsesFrameworkImplInAClientLib, doesNotAcceptForeignCreatedClientsForDestruction)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw1{config};
        RamsesFramework fw2{config};

        auto client1 = fw1.createClient("first client");
        auto client2 = fw2.createClient("second client");
        EXPECT_FALSE(fw2.destroyClient(*client1));
        EXPECT_FALSE(fw1.destroyClient(*client2));
    }

    TEST(ARamsesFrameworkImplInAClientLib, doesNotAcceptSameClientTwiceForDestruction)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw{config};

        auto client = fw.createClient("client");
        EXPECT_TRUE(fw.destroyClient(*client));
        EXPECT_FALSE(fw.destroyClient(*client));
    }

    TEST(ARamsesFrameworkImplInAClientLib, canCreateDestroyAndRecreateAClient)
    {
        RamsesFrameworkConfig config{EFeatureLevel_Latest};
        RamsesFramework fw{config};

        auto client = fw.createClient("client");
        EXPECT_NE(nullptr, client);
        EXPECT_TRUE(fw.destroyClient(*client));
        client = fw.createClient("client");
        EXPECT_NE(nullptr, client);
    }
}
