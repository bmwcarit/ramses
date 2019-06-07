//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/WarpingMeshData.h"

#include "RamsesFrameworkImpl.h"
#include "RamsesRendererImpl.h"
#include "Components/ISceneGraphProviderComponent.h"
#include "Components/FlushTimeInformation.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/DisplayEventHandler.h"
#include "Scene/ClientScene.h"
#include "RendererEventTestHandler.h"
#include "RamsesRendererUtils.h"

//This is needed to abstract from a specific rendering platform
#include "PlatformFactoryMock.h"
#include "Platform_Base/PlatformFactory_Base.h"

namespace ramses_internal
{
    using namespace testing;

    NiceMock<PlatformFactoryNiceMock>* gPlatformFactoryMock = NULL;

    ramses_internal::IPlatformFactory* ramses_internal::PlatformFactory_Base::CreatePlatformFactory(const ramses_internal::RendererConfig&)
    {
        gPlatformFactoryMock = new ::testing::NiceMock<PlatformFactoryNiceMock>();
        return gPlatformFactoryMock;
    }

    class ARamsesRendererDispatch : public ::testing::Test
    {
    protected:
        ARamsesRendererDispatch()
            : m_framework()
            , m_renderer(m_framework, ramses::RendererConfig())
            , m_sceneId(33u)
            , m_scene(SceneInfo(SceneId(m_sceneId)))
            , m_sceneGraphProvider(m_framework.impl.getScenegraphComponent())
            , m_sceneGraphSender(m_framework.impl.getScenegraphComponent())
        {
        }

        ramses::displayId_t addDisplay(bool warpingEnabled = false)
        {
            //Create a display
            ramses::DisplayConfig displayConfig;
            if (warpingEnabled)
            {
                displayConfig.enableWarpingPostEffect();
            }
            EXPECT_EQ(ramses::StatusOK, displayConfig.validate());
            return m_renderer.createDisplay(displayConfig);
        }

        ramses::displayId_t createDisplayAndExpectResult(ramses::ERendererEventResult expectedResult = ramses::ERendererEventResult_OK)
        {
            const ramses::displayId_t m_displayId = addDisplay();
            updateAndDispatch(m_handler);
            m_handler.expectDisplayCreated(m_displayId, expectedResult);

            return m_displayId;
        }

        void doUpdateLoop()
        {
            ramses::RamsesRendererUtils::DoOneLoop(m_renderer.impl.getRenderer(), ramses_internal::ELoopMode_UpdateOnly, std::chrono::microseconds{ 0u });
        }

        void updateAndDispatch(ramses::IRendererEventHandler& eventHandler, uint32_t loops = 1u)
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.flush());
            for (uint32_t i = 0u; i < loops; ++i)
                doUpdateLoop();
            EXPECT_EQ(ramses::StatusOK, m_renderer.dispatchEvents(eventHandler));
        }

        void publishScene()
        {
            m_sceneGraphProvider.handleCreateScene(m_scene, false);
            m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(m_sceneId), EScenePublicationMode_LocalOnly);
        }

        void unpublishScene()
        {
            m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(m_sceneId));
        }

        void subscribeScene(ramses::sceneId_t newSceneID)
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.subscribeScene(newSceneID));
            updateAndDispatch(m_handler);

            // receive m_scene
            m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(newSceneID)), EScenePublicationMode_LocalOnly);

            // receive initial flush
            SceneActionCollection initialSceneActions;
            SceneActionCollectionCreator creator(initialSceneActions);
            creator.flush(1u, false, true, ramses_internal::SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(initialSceneActions), ramses_internal::SceneId(newSceneID), EScenePublicationMode_LocalOnly);
        }

        void mapScene(ramses::sceneId_t sceneIdToMap, ramses::displayId_t m_displayId, bool expectSuccess = true)
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.mapScene(m_displayId, sceneIdToMap));
            updateAndDispatch(m_handler, 2u);
            m_handler.expectSceneMapped(sceneIdToMap, expectSuccess ? ramses::ERendererEventResult_OK : ramses::ERendererEventResult_FAIL);
        }

        void namedSceneFlush(ramses::sceneId_t newSceneId, ramses::sceneVersionTag_t versionTag)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            const ramses_internal::SceneVersionTag internalVersionTag(versionTag);
            creator.flush(1u, false, false, {}, {}, {}, internalVersionTag);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(newSceneId), EScenePublicationMode_LocalOnly);
        }

        void flushWithTimeInfo(ramses::sceneId_t newSceneId, FlushTime::Clock::time_point timeStamp, std::chrono::milliseconds limit)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            const FlushTimeInformation timeInfo{ limit.count() > 0u ? timeStamp + limit : FlushTime::InvalidTimestamp, {} };
            creator.flush(1u, false, false, {}, {}, timeInfo);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(newSceneId), EScenePublicationMode_LocalOnly);
        }

        void createPublishedAndSubscribedScene(ramses::sceneId_t newSceneId, ramses_internal::ClientScene& newScene)
        {
            m_sceneGraphProvider.handleCreateScene(newScene, false);
            m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(newSceneId), EScenePublicationMode_LocalOnly);
            updateAndDispatch(m_handler);
            m_handler.expectScenePublished(newSceneId);

            subscribeScene(newSceneId);
            updateAndDispatch(m_handler);
            m_handler.expectSceneSubscribed(newSceneId, ramses::ERendererEventResult_OK);
        }

        void createPublishedAndSubscribedAndMappedScene(ramses::sceneId_t newSceneId, ramses_internal::ClientScene& newScene, ramses::displayId_t m_displayId)
        {
            createPublishedAndSubscribedScene(newSceneId, newScene);
            mapScene(newSceneId, m_displayId);
        }

    protected:
        ramses::RamsesFramework m_framework;
        ramses::RamsesRenderer m_renderer;
        RendererEventTestHandler m_handler;

        const ramses::sceneId_t m_sceneId;
        ramses_internal::ClientScene m_scene;

        ramses_internal::ISceneGraphProviderComponent& m_sceneGraphProvider;
        ramses_internal::ISceneGraphSender& m_sceneGraphSender;
    };

    class ARamsesRendererDispatchWithProviderConsumerScenes : public ARamsesRendererDispatch
    {
    public:
        ARamsesRendererDispatchWithProviderConsumerScenes()
            : ARamsesRendererDispatch()
            , m_dataProviderHandle(2u)
            , m_dataConsumerHandle(3u)
            , m_dataProviderId(112u)
            , m_dataConsumerId(113u)
            , m_sceneProviderId(77u)
            , m_sceneConsumerId(78u)
            , m_sceneProvider(SceneInfo(SceneId(m_sceneProviderId)))
            , m_sceneConsumer(SceneInfo(SceneId(m_sceneConsumerId)))
            , m_displayId(ramses::InvalidDisplayId)
        {
            m_displayId = createDisplayAndExpectResult();

            createPublishedAndSubscribedAndMappedScene(m_sceneProviderId, m_sceneProvider, m_displayId);
            createPublishedAndSubscribedAndMappedScene(m_sceneConsumerId, m_sceneConsumer, m_displayId);
        }

    protected:
        void createTransformationDataSlotProvider(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::NodeHandle node(7u);
            const ramses_internal::DataInstanceHandle dataRef;
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateNode(0u, node);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, ramses_internal::DataSlotId(m_dataProviderId), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void createTransformationDataSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::NodeHandle node(8u);
            const ramses_internal::DataInstanceHandle dataRef;
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateNode(0u, node);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, ramses_internal::DataSlotId(m_dataConsumerId), node, dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataConsumerHandle);
            creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void createDataSlotProvider(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::DataLayoutHandle dataLayout(0u);
            const ramses_internal::DataInstanceHandle dataRef(3u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, dataLayout);
            creator.allocateDataInstance(dataLayout, dataRef);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_DataProvider, ramses_internal::DataSlotId(m_dataProviderId), ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void createDataSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::DataLayoutHandle dataLayout(0u);
            const ramses_internal::DataInstanceHandle dataRef(3u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, dataLayout);
            creator.allocateDataInstance(dataLayout, dataRef);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_DataConsumer, ramses_internal::DataSlotId(m_dataConsumerId), ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataConsumerHandle);
            creator.flush(1u, false, false, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void createTextureSlotProvider(ramses::sceneId_t targetSceneId)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TextureProvider, ramses_internal::DataSlotId(m_dataProviderId), ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle(), ramses_internal::ResourceContentHash(0x1234, 0), ramses_internal::TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, false, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void createTextureSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::TextureSamplerHandle sampler(4u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateTextureSampler({ {}, ResourceContentHash(1, 2) }, sampler);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TextureConsumer, ramses_internal::DataSlotId(m_dataConsumerId), ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle(), ramses_internal::ResourceContentHash::Invalid(), sampler }, m_dataConsumerHandle);
            creator.flush(1u, false, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        void destroyDataSlot(ramses::sceneId_t targetSceneId, ramses_internal::DataSlotHandle dataSlotHandle)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.releaseDataSlot(dataSlotHandle);
            creator.flush(1u, false, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId), EScenePublicationMode_LocalOnly);
        }

        const ramses_internal::DataSlotHandle m_dataProviderHandle;
        const ramses_internal::DataSlotHandle m_dataConsumerHandle;
        const ramses::dataProviderId_t m_dataProviderId;
        const ramses::dataConsumerId_t m_dataConsumerId;

        const ramses::sceneId_t m_sceneProviderId;
        const ramses::sceneId_t m_sceneConsumerId;
        ramses_internal::ClientScene m_sceneProvider;
        ramses_internal::ClientScene m_sceneConsumer;

        ramses::displayId_t m_displayId;
    };

    TEST_F(ARamsesRendererDispatch, generatesEventForScenePublish)
    {
        publishScene();
        updateAndDispatch(m_handler);
        m_handler.expectScenePublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneUnpublish)
    {
        publishScene();
        updateAndDispatch(m_handler);
        m_handler.expectScenePublished(m_sceneId);

        unpublishScene();
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnpublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventAfterApplyingFlushWithVersionTag)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        const ramses::sceneVersionTag_t versionTag{ 333 };
        namedSceneFlush(m_sceneId, versionTag);
        updateAndDispatch(m_handler);
        m_handler.expectSceneFlushed(m_sceneId, versionTag, ramses::ESceneResourceStatus_Pending);
    }

    TEST_F(ARamsesRendererDispatch, generatesTwoEventsAfterApplyingTwoFlushesWithDifferentVersionTag)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        const ramses::sceneVersionTag_t versionTag1{ 333 };
        const ramses::sceneVersionTag_t versionTag2{ 444 };
        namedSceneFlush(m_sceneId, versionTag1);
        namedSceneFlush(m_sceneId, versionTag2);
        updateAndDispatch(m_handler);
        m_handler.expectSceneFlushed(m_sceneId, versionTag1, ramses::ESceneResourceStatus_Pending, 2u);
        m_handler.expectSceneFlushed(m_sceneId, versionTag2, ramses::ESceneResourceStatus_Pending, 1u);
    }

    TEST_F(ARamsesRendererDispatch, generatesTwoEventsAfterApplyingTwoFlushesWithSameVersionTag)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        const ramses::sceneVersionTag_t versionTag1{ 333 };
        const ramses::sceneVersionTag_t versionTag2{ 333 };
        namedSceneFlush(m_sceneId, versionTag1);
        namedSceneFlush(m_sceneId, versionTag2);
        updateAndDispatch(m_handler);
        m_handler.expectSceneFlushed(m_sceneId, versionTag1, ramses::ESceneResourceStatus_Pending, 2u);
        m_handler.expectSceneFlushed(m_sceneId, versionTag2, ramses::ESceneResourceStatus_Pending, 1u);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventsOnlyAfterApplyingFlushesWithValidVersionTag)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        const ramses::sceneVersionTag_t versionTag1{ 333 };
        const ramses::sceneVersionTag_t versionTag2{ 444 };
        const ramses::sceneVersionTag_t versionTag3{ ramses::InvalidSceneVersionTag };
        const ramses::sceneVersionTag_t versionTag4{ 555 };
        const ramses::sceneVersionTag_t versionTag5{ 555 };
        namedSceneFlush(m_sceneId, versionTag1);
        namedSceneFlush(m_sceneId, versionTag2);
        namedSceneFlush(m_sceneId, versionTag3);
        namedSceneFlush(m_sceneId, versionTag4);
        namedSceneFlush(m_sceneId, versionTag5);
        updateAndDispatch(m_handler);
        m_handler.expectSceneFlushed(m_sceneId, versionTag1, ramses::ESceneResourceStatus_Pending, 4u);
        m_handler.expectSceneFlushed(m_sceneId, versionTag2, ramses::ESceneResourceStatus_Pending, 3u);
        // no event for invalid version tag
        m_handler.expectSceneFlushed(m_sceneId, versionTag4, ramses::ESceneResourceStatus_Pending, 2u);
        m_handler.expectSceneFlushed(m_sceneId, versionTag5, ramses::ESceneResourceStatus_Pending, 1u);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneExpired)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        flushWithTimeInfo(m_sceneId, FlushTime::Clock::time_point(std::chrono::milliseconds(1u)), std::chrono::milliseconds(1u));
        updateAndDispatch(m_handler);
        m_handler.expectSceneExpired(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneRecoveredFromExpired)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        flushWithTimeInfo(m_sceneId, FlushTime::Clock::time_point(std::chrono::milliseconds(1u)), std::chrono::milliseconds(1u));
        updateAndDispatch(m_handler);
        m_handler.expectSceneExpired(m_sceneId);

        flushWithTimeInfo(m_sceneId, FlushTime::Clock::now(), std::chrono::hours(1));
        updateAndDispatch(m_handler);
        m_handler.expectSceneRecoveredFromExpiration(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneSubscribe)
    {
        publishScene();
        updateAndDispatch(m_handler);
        m_handler.expectScenePublished(m_sceneId);

        subscribeScene(m_sceneId);
        updateAndDispatch(m_handler);
        m_handler.expectSceneSubscribed(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneSubscribe)
    {
        subscribeScene(m_sceneId);
        updateAndDispatch(m_handler);
        m_handler.expectSceneSubscribed(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneUnsubscribe)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unsubscribeScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneUnsubscribe)
    {
        EXPECT_EQ(ramses::StatusOK, m_renderer.unsubscribeScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesINDIRECTEventForSceneUnsubscribeAfterSceneUnpublished)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        unpublishScene();
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneMap)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedScene(m_sceneId, m_scene);
        mapScene(m_sceneId, m_displayId);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForMapOfUnknownScene)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        mapScene(m_sceneId, m_displayId, false);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneMapOnInvalidDisplay)
    {
        const ramses::displayId_t m_displayId(1);
        mapScene(m_sceneId, m_displayId, false);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneUnmap)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();

        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        mapScene(m_sceneId, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneUnmap)
    {
        EXPECT_EQ(ramses::StatusOK, m_renderer.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesINDIRECTEventForSceneUnmapAfterSceneUnpublished)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();

        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        mapScene(m_sceneId, m_displayId);

        unpublishScene();
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneShow)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneShow)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        EXPECT_EQ(ramses::StatusOK, m_renderer.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneHide)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.hideScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneHidden(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneHide)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.hideScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneHidden(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesINDIRECTEventForSceneHideAfterSceneUnpublished)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_OK);

        unpublishScene();
        updateAndDispatch(m_handler);
        m_handler.expectSceneHidden(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForDisplayCreation)
    {
        createDisplayAndExpectResult();
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForDisplayCreation)
    {
        ON_CALL(*gPlatformFactoryMock, createRenderBackend(_, _)).WillByDefault(Return(static_cast<ramses_internal::IRenderBackend*>(NULL)));
        createDisplayAndExpectResult(ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForDisplayDestruction)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(m_displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(m_displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForInvalidDisplayDestruction)
    {
        const ramses::displayId_t m_displayId(1u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(m_displayId));

        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(m_displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForDisplayDestructionIfAnySceneMappedToIt)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(m_displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(m_displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForDisplayDestructionAfterFailedAttemptDueToMappedScene)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(m_displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(m_displayId, ramses::ERendererEventResult_FAIL);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(m_displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(m_displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWarpingMeshDataUpdate)
    {
        ramses::displayId_t m_displayId = addDisplay(true);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(m_displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(m_displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(m_displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForWarpingMeshDataUpdateOfInvalidDisplay)
    {
        const ramses::displayId_t invalidDisplay(1u);
        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(invalidDisplay, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(invalidDisplay, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWarpingMeshDataUpdateOfDisplayWithoutWarping)
    {
        ramses::displayId_t m_displayId = addDisplay(false);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(m_displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(m_displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(m_displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferCreation)
    {
        ramses::displayId_t m_displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(m_displayId, ramses::ERendererEventResult_OK);

        const ramses::offscreenBufferId_t bufferId = m_renderer.createOffscreenBuffer(m_displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(m_displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferCreation)
    {
        ramses::displayId_t m_displayId(0u);
        const ramses::offscreenBufferId_t bufferId = m_renderer.createOffscreenBuffer(m_displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(m_displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t m_displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(m_displayId, ramses::ERendererEventResult_OK);

        const ramses::offscreenBufferId_t bufferId = m_renderer.createOffscreenBuffer(m_displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(m_displayId, bufferId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(m_displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(m_displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t m_displayId(0u);
        const ramses::offscreenBufferId_t bufferId(0u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(m_displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(m_displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneAssignedToOffscreenBuffer)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        const ramses::offscreenBufferId_t bufferId = m_renderer.createOffscreenBuffer(m_displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(m_displayId, bufferId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.assignSceneToOffscreenBuffer(m_sceneId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToOffscreenBuffer(m_sceneId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneAssignedToOffscreenBuffer)
    {
        const ramses::offscreenBufferId_t bufferId(2u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.assignSceneToOffscreenBuffer(m_sceneId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToOffscreenBuffer(m_sceneId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneAssignedToFramebuffer)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, m_displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.assignSceneToFramebuffer(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToFramebuffer(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneAssignedToFramebuffer)
    {
        EXPECT_EQ(ramses::StatusOK, m_renderer.assignSceneToFramebuffer(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToFramebuffer(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowClosed)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(m_displayId);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onClose();
        updateAndDispatch(m_handler);
        m_handler.expectWindowClosed(m_displayId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowResized)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(m_displayId);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onResize(100, 200);
        updateAndDispatch(m_handler);
        m_handler.expectWindowResized(m_displayId, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyPressed)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(m_displayId);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Pressed,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_E);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(m_displayId, ramses::EKeyEvent_Pressed, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_E);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyReleased)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(m_displayId);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Released,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_F);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(m_displayId, ramses::EKeyEvent_Released, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_F);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventsForMouseActionsOnDisplay)
    {
        const ramses::displayId_t m_displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(m_displayId);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_RightButtonDown, 20, 30);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_RightButtonUp, 21, 31);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_LeftButtonDown, 22, 32);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_LeftButtonUp, 23, 33);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_MiddleButtonDown, 24, 34);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_MiddleButtonUp, 25, 35);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WheelDown, 26, 36);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WheelUp, 27, 37);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_Move, 28, 38);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WindowEnter, 29, 39);
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onMouseEvent(ramses_internal::EMouseEventType_WindowLeave, 30, 40);
        updateAndDispatch(m_handler);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_RightButtonDown, 20, 30);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_RightButtonUp, 21, 31);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_LeftButtonDown, 22, 32);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_LeftButtonUp, 23, 33);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_MiddleButtonDown, 24, 34);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_MiddleButtonUp, 25, 35);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_WheelDown, 26, 36);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_WheelUp, 27, 37);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_Move, 28, 38);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_WindowEnter, 29, 39);
        m_handler.expectMouseEvent(m_displayId, ramses::EMouseEvent_WindowLeave, 30, 40);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTransformationProviderCreation)
    {
        createTransformationDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTransformationConsumerCreation)
    {
        createTransformationDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTransformationProviderDestruction)
    {
        createTransformationDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        destroyDataSlot(m_sceneProviderId, m_dataProviderHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderDestroyed(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTransformationConsumerDestruction)
    {
        createTransformationDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }
    //
    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForDataProviderCreation)
    {
        createDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForDataConsumerCreation)
    {
        createDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForDataProviderDestruction)
    {
        createDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        destroyDataSlot(m_sceneProviderId, m_dataProviderHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderDestroyed(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForDataConsumerDestruction)
    {
        createDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }
    //
    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTextureProviderCreation)
    {
        createTextureSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTextureConsumerCreation)
    {
        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTextureProviderDestruction)
    {
        createTextureSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        destroyDataSlot(m_sceneProviderId, m_dataProviderHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderDestroyed(m_sceneProviderId, m_dataProviderId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesEventForTextureConsumerDestruction)
    {
        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForTransformationLink)
    {
        createTransformationDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTransformationDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForLinkWithWrongParameters)
    {
        const ramses::sceneId_t invalidSceneProviderId(777u);
        const ramses::sceneId_t invalidSceneConsumerId(778u);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(invalidSceneProviderId, m_dataProviderId, invalidSceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(invalidSceneProviderId, m_dataProviderId, invalidSceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForTransformationUnlink)
    {
        createTransformationDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTransformationDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unlinkData(m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForUnlinkWithWrongParameters)
    {
        const ramses::sceneId_t invalidSceneConsumerId(778u);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unlinkData(invalidSceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(invalidSceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesINDIRECTEventForTransformationUnlinkAfterDataSlotRemovedFromScene)
    {
        createTransformationDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTransformationDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);

        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForDataLink)
    {
        createDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForDataUnlink)
    {
        createDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unlinkData(m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesINDIRECTEventForDataUnlinkAfterDataSlotRemovedFromScene)
    {
        createDataSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createDataSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);

        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForTextureLink)
    {
        createTextureSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForTextureUnlink)
    {
        createTextureSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unlinkData(m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesINDIRECTEventForTextureUnlinkAfterDataSlotRemovedFromScene)
    {
        createTextureSlotProvider(m_sceneProviderId);
        updateAndDispatch(m_handler);
        m_handler.expectDataProviderCreated(m_sceneProviderId, m_dataProviderId);

        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);

        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForOffscreenBufferLink)
    {
        const ramses::offscreenBufferId_t bufferId = m_renderer.createOffscreenBuffer(m_displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(m_displayId, bufferId, ramses::ERendererEventResult_OK);

        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkOffscreenBufferToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferLinkedToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForOffscreenBufferLink)
    {
        const ramses::offscreenBufferId_t bufferId(1u);

        EXPECT_EQ(ramses::StatusOK, m_renderer.linkOffscreenBufferToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferLinkedToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, confidenceTest_generatesAppropriateEventsForFullCycleOfSceneWithMapAndDataLink_commandsBatched)
    {
        const ramses::sceneId_t customSceneProviderId(777u);
        const ramses::sceneId_t customsceneConsumerId(778u);
        const ramses_internal::SceneId sceneProviderIdInternal(customSceneProviderId);
        const ramses_internal::SceneId sceneConsumerIdInternal(customsceneConsumerId);
        const SceneInfo createInfoProvider(sceneProviderIdInternal);
        const SceneInfo createInfoConsumer(sceneConsumerIdInternal);
        ramses_internal::ClientScene sceneProvider(createInfoProvider);
        ramses_internal::ClientScene sceneConsumer(createInfoConsumer);

        // publish scenes
        m_sceneGraphProvider.handleCreateScene(sceneProvider, false);
        m_sceneGraphProvider.handleCreateScene(sceneConsumer, false);
        m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(customSceneProviderId), EScenePublicationMode_LocalOnly);
        m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(customsceneConsumerId), EScenePublicationMode_LocalOnly);
        updateAndDispatch(m_handler);
        m_handler.expectScenePublished(customSceneProviderId);
        m_handler.expectScenePublished(customsceneConsumerId);

        // subscribe and emulate receive scenes
        EXPECT_EQ(ramses::StatusOK, m_renderer.subscribeScene(customSceneProviderId));
        EXPECT_EQ(ramses::StatusOK, m_renderer.subscribeScene(customsceneConsumerId));
        updateAndDispatch(m_handler); // needed to process subscription request
        m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(customSceneProviderId)), EScenePublicationMode_LocalOnly);
        m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(customsceneConsumerId)), EScenePublicationMode_LocalOnly);

        // create data slots
        const ramses_internal::NodeHandle node(7u);
        const ramses_internal::DataInstanceHandle dataRef;
        SceneActionCollection sceneActions;
        SceneActionCollectionCreator creator(sceneActions);

        creator.allocateNode(0u, node);
        creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, ramses_internal::DataSlotId(m_dataProviderId), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataProviderHandle);
        creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, sceneActions.copy(), ramses_internal::SceneId(customSceneProviderId), EScenePublicationMode_LocalOnly);
        sceneActions.clear();

        creator.allocateNode(0u, node);
        creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, ramses_internal::DataSlotId(m_dataConsumerId), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataConsumerHandle);
        creator.flush(1u, false, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, sceneActions.copy(), ramses_internal::SceneId(customsceneConsumerId), EScenePublicationMode_LocalOnly);

        // create data link
        m_renderer.flush();
        doUpdateLoop();
        EXPECT_EQ(ramses::StatusOK, m_renderer.linkData(customSceneProviderId, m_dataProviderId, customsceneConsumerId, m_dataConsumerId));

        // create display
        const ramses::displayId_t display = addDisplay();

        // map scenes
        EXPECT_EQ(ramses::StatusOK, m_renderer.mapScene(display, customSceneProviderId));
        EXPECT_EQ(ramses::StatusOK, m_renderer.mapScene(display, customsceneConsumerId));

        // cleanup
        m_renderer.flush(); // emulation of client commands via sceneGraphProvider go directly to m_renderer command buffer, to keep order of commands previous m_renderer API calls must be flushed
        doUpdateLoop(); // needed for successfully processing mapped commands
        doUpdateLoop(); // needed for successfully processing mapped commands

        m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(customSceneProviderId));
        m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(customsceneConsumerId));
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(display));

        // process commands
        updateAndDispatch(m_handler);
        m_handler.expectSceneSubscribed(customSceneProviderId, ramses::ERendererEventResult_OK);
        m_handler.expectSceneSubscribed(customsceneConsumerId, ramses::ERendererEventResult_OK);
        m_handler.expectDataConsumerCreated(customsceneConsumerId, m_dataConsumerId);
        m_handler.expectDataProviderCreated(customSceneProviderId, m_dataProviderId);
        m_handler.expectDataLinked(customSceneProviderId, m_dataProviderId, customsceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
        m_handler.expectDisplayCreated(display, ramses::ERendererEventResult_OK);
        m_handler.expectSceneMapped(customSceneProviderId, ramses::ERendererEventResult_OK, 2u); // to be found within the first 2 events of the queue, because mapping order is not deterministic
        m_handler.expectSceneMapped(customsceneConsumerId, ramses::ERendererEventResult_OK);
        m_handler.expectSceneUnmapped(customSceneProviderId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnsubscribed(customSceneProviderId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(customSceneProviderId);
        m_handler.expectSceneUnmapped(customsceneConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnsubscribed(customsceneConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(customsceneConsumerId);
        m_handler.expectDisplayDestroyed(display, ramses::ERendererEventResult_OK);

        // events for unlink and removal of data slots do not get generated when the whole m_scene is unpublished
    }

    TEST_F(ARamsesRendererDispatch, canInvokeRendererCommandsinEventHandler)
    {
        // RendererEventTestHandler, which maps a m_scene when it gets published.
        // All necessary m_renderer commands are called within the m_handler methods.
        class RecursiveRendererEventTestHandler : public RendererEventTestHandler
        {
        public:
            RecursiveRendererEventTestHandler(ramses::RamsesRenderer& renderer, const ramses::sceneId_t& sceneId,
                                              const ramses::displayId_t& displayId, ramses_internal::ISceneGraphSender& sceneGraphSender, ramses::RamsesFramework& framework)
                : m_renderer(renderer)
                , m_sceneId(sceneId)
                , m_displayId(displayId)
                , m_sceneGraphSender(sceneGraphSender)
                , m_framework(framework)
            {
            }

            void updateAndDispatch()
            {
                m_renderer.flush();
                ramses::RamsesRendererUtils::DoOneLoop(m_renderer.impl.getRenderer(), ramses_internal::ELoopMode_UpdateOnly, std::chrono::microseconds{ 0u });
                m_renderer.dispatchEvents(*this);
            }

            virtual void scenePublished(ramses::sceneId_t scnId) override
            {
                RendererEventTestHandler::scenePublished(scnId);
                if (scnId == m_sceneId)
                {
                    EXPECT_EQ(ramses::StatusOK, m_renderer.subscribeScene(m_sceneId));
                    updateAndDispatch();
                    // receive m_scene
                    m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(m_sceneId)), EScenePublicationMode_LocalOnly);
                    // receive initial flush
                    SceneActionCollection initialSceneActions;
                    SceneActionCollectionCreator creator(initialSceneActions);
                    creator.flush(1u, false, false);
                    m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(initialSceneActions), ramses_internal::SceneId(m_sceneId), EScenePublicationMode_LocalOnly);
                    updateAndDispatch();
                }
            }

            virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
            {
                RendererEventTestHandler::sceneSubscribed(sceneId, result);
                if (sceneId == m_sceneId && result == ramses::ERendererEventResult_OK)
                {
                    EXPECT_EQ(ramses::StatusOK, m_renderer.mapScene(m_displayId, m_sceneId));
                    updateAndDispatch();
                    updateAndDispatch();
                }
            }

        private:
            ramses::RamsesRenderer& m_renderer;
            ramses::sceneId_t m_sceneId;
            ramses::displayId_t m_displayId;
            ramses_internal::ISceneGraphSender& m_sceneGraphSender;
            ramses::RamsesFramework& m_framework;
        };

        //create display first
        ramses::displayId_t m_displayId = addDisplay(false);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(m_displayId, ramses::ERendererEventResult_OK);

        //m_scene publishing should result in mapped Scene after dispatching events to the recursive m_handler
        RecursiveRendererEventTestHandler recHandler(m_renderer, m_sceneId, m_displayId, m_sceneGraphSender, m_framework);
        publishScene();
        updateAndDispatch(recHandler);
        recHandler.expectScenePublished(m_sceneId);
        recHandler.expectSceneSubscribed(m_sceneId, ramses::ERendererEventResult_OK);
        recHandler.expectSceneMapped(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.unmapScene(m_sceneId));
        EXPECT_EQ(ramses::StatusOK, m_renderer.unsubscribeScene(m_sceneId));
        updateAndDispatch(recHandler);
        recHandler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);
        recHandler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_OK);
    }

}
