//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/RendererSceneControl_legacy.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/WarpingMeshData.h"

#include "RamsesFrameworkImpl.h"
#include "RamsesRendererImpl.h"
#include "Components/ISceneGraphProviderComponent.h"
#include "Components/FlushTimeInformation.h"
#include "ComponentMocks.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/DisplayEventHandler.h"
#include "RendererLib/EKeyModifier.h"
#include "Scene/ClientScene.h"
#include "RendererEventTestHandler.h"
#include "RamsesRendererUtils.h"

//This is needed to abstract from a specific rendering platform
#include "PlatformFactoryMock.h"
#include "Platform_Base/PlatformFactory_Base.h"
#include "ResourceProviderMock.h"

namespace ramses_internal
{
    using namespace testing;

    NiceMock<PlatformFactoryNiceMock>* gPlatformFactoryMock = nullptr;

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
            , m_renderer(*m_framework.createRenderer(ramses::RendererConfig()))
            , m_sceneControlAPI(*m_renderer.getSceneControlAPI_legacy())
            , m_sceneId(33u)
            , m_scene(SceneInfo(SceneId(m_sceneId.getValue())))
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
            const ramses::displayId_t displayId = addDisplay();
            updateAndDispatch(m_handler);
            m_handler.expectDisplayCreated(displayId, expectedResult);

            return displayId;
        }

        void doUpdateLoop()
        {
            m_renderer.impl.getRenderer().doOneLoop(ramses_internal::ELoopMode::UpdateOnly);
        }

        void updateAndDispatch(RendererEventTestHandler& eventHandler, uint32_t loops = 1u)
        {
            EXPECT_EQ(ramses::StatusOK, m_renderer.flush());
            EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.flush());
            for (uint32_t i = 0u; i < loops; ++i)
                doUpdateLoop();
            EXPECT_EQ(ramses::StatusOK, m_renderer.dispatchEvents(eventHandler));
            EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.dispatchEvents(eventHandler));
        }

        void publishScene()
        {
            m_sceneGraphProvider.handleCreateScene(m_scene, false, m_eventConsumer);
            m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(m_sceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void unpublishScene()
        {
            m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(m_sceneId.getValue()));
        }

        void subscribeScene(ramses::sceneId_t newSceneID)
        {
            EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.subscribeScene(newSceneID));
            updateAndDispatch(m_handler);

            // receive m_scene
            m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(newSceneID.getValue())), EScenePublicationMode_LocalOnly);

            // receive initial flush
            SceneActionCollection initialSceneActions;
            SceneActionCollectionCreator creator(initialSceneActions);
            creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(initialSceneActions), ramses_internal::SceneId(newSceneID.getValue()), EScenePublicationMode_LocalOnly);
        }

        void mapScene(ramses::sceneId_t sceneIdToMap, ramses::displayId_t displayId, bool expectSuccess = true)
        {
            EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.mapScene(displayId, sceneIdToMap));
            updateAndDispatch(m_handler, 2u);
            m_handler.expectSceneMapped(sceneIdToMap, expectSuccess ? ramses::ERendererEventResult_OK : ramses::ERendererEventResult_FAIL);
        }

        void namedSceneFlush(ramses::sceneId_t newSceneId, ramses::sceneVersionTag_t versionTag)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            const ramses_internal::SceneVersionTag internalVersionTag(versionTag);
            creator.flush(1u, false, {}, {}, {}, {}, internalVersionTag);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(newSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void flushWithTimeInfo(ramses::sceneId_t newSceneId, FlushTime::Clock::time_point timeStamp, std::chrono::milliseconds limit)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            const FlushTimeInformation timeInfo{ limit.count() > 0u ? timeStamp + limit : FlushTime::InvalidTimestamp, {} };
            creator.flush(1u, false, {}, {}, {}, timeInfo);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(newSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }
        void createPublishedAndSubscribedScene(ramses::sceneId_t newSceneId, ramses_internal::ClientScene& newScene)
        {
            m_sceneGraphProvider.handleCreateScene(newScene, false, m_eventConsumer);
            m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(newSceneId.getValue()), EScenePublicationMode_LocalOnly);
            updateAndDispatch(m_handler);
            m_handler.expectScenePublished(newSceneId);

            subscribeScene(newSceneId);
            updateAndDispatch(m_handler);
            m_handler.expectSceneSubscribed(newSceneId, ramses::ERendererEventResult_OK);
        }

        void createPublishedAndSubscribedAndMappedScene(ramses::sceneId_t newSceneId, ramses_internal::ClientScene& newScene, ramses::displayId_t displayId)
        {
            createPublishedAndSubscribedScene(newSceneId, newScene);
            mapScene(newSceneId, displayId);
        }

    protected:
        ramses::RamsesFramework m_framework;
        ramses::RamsesRenderer& m_renderer;
        ramses::RendererSceneControl_legacy& m_sceneControlAPI;
        RendererEventTestHandler m_handler;

        const ramses::sceneId_t m_sceneId;
        ramses_internal::ClientScene m_scene;

        ramses_internal::ISceneGraphProviderComponent& m_sceneGraphProvider;
        ramses_internal::ISceneGraphSender& m_sceneGraphSender;
        StrictMock<SceneProviderEventConsumerMock> m_eventConsumer;
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
            , m_sceneProvider(SceneInfo(SceneId(m_sceneProviderId.getValue())))
            , m_sceneConsumer(SceneInfo(SceneId(m_sceneConsumerId.getValue())))
        {
            displayId = createDisplayAndExpectResult();

            createPublishedAndSubscribedAndMappedScene(m_sceneProviderId, m_sceneProvider, displayId);
            createPublishedAndSubscribedAndMappedScene(m_sceneConsumerId, m_sceneConsumer, displayId);
        }

    protected:
        void createTransformationDataSlotProvider(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::NodeHandle node(7u);
            const ramses_internal::DataInstanceHandle dataRef;
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateNode(0u, node);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, ramses_internal::DataSlotId(m_dataProviderId.getValue()), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void createTransformationDataSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::NodeHandle node(8u);
            const ramses_internal::DataInstanceHandle dataRef;
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateNode(0u, node);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, ramses_internal::DataSlotId(m_dataConsumerId.getValue()), node, dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataConsumerHandle);
            creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u,10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void createDataSlotProvider(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::DataLayoutHandle dataLayout(0u);
            const ramses_internal::DataInstanceHandle dataRef(3u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ramses_internal::ResourceProviderMock::FakeEffectHash, dataLayout);
            creator.allocateDataInstance(dataLayout, dataRef);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_DataProvider, ramses_internal::DataSlotId(m_dataProviderId.getValue()), ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void createDataSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::DataLayoutHandle dataLayout(0u);
            const ramses_internal::DataInstanceHandle dataRef(3u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataLayout({ DataFieldInfo(EDataType_Float) }, ramses_internal::ResourceProviderMock::FakeEffectHash, dataLayout);
            creator.allocateDataInstance(dataLayout, dataRef);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_DataConsumer, ramses_internal::DataSlotId(m_dataConsumerId.getValue()), ramses_internal::NodeHandle(), dataRef, ramses_internal::ResourceContentHash::Invalid(), ramses_internal::TextureSamplerHandle() }, m_dataConsumerHandle);
            creator.flush(1u, false, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void createTextureSlotProvider(ramses::sceneId_t targetSceneId)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TextureProvider, ramses_internal::DataSlotId(m_dataProviderId.getValue()), ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle(), ramses_internal::ResourceContentHash(0x1234, 0), ramses_internal::TextureSamplerHandle() }, m_dataProviderHandle);
            creator.flush(1u, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void createTextureSlotConsumer(ramses::sceneId_t targetSceneId)
        {
            const ramses_internal::TextureSamplerHandle sampler(4u);
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.allocateTextureSampler({ {}, ResourceContentHash(1, 2) }, sampler);
            creator.allocateDataSlot({ ramses_internal::EDataSlotType_TextureConsumer, ramses_internal::DataSlotId(m_dataConsumerId.getValue()), ramses_internal::NodeHandle(), ramses_internal::DataInstanceHandle(), ramses_internal::ResourceContentHash::Invalid(), sampler }, m_dataConsumerHandle);
            creator.flush(1u, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        void destroyDataSlot(ramses::sceneId_t targetSceneId, ramses_internal::DataSlotHandle dataSlotHandle)
        {
            SceneActionCollection sceneActions;
            SceneActionCollectionCreator creator(sceneActions);
            creator.releaseDataSlot(dataSlotHandle);
            creator.flush(1u, false);
            m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(targetSceneId.getValue()), EScenePublicationMode_LocalOnly);
        }

        const ramses_internal::DataSlotHandle m_dataProviderHandle;
        const ramses_internal::DataSlotHandle m_dataConsumerHandle;
        const ramses::dataProviderId_t m_dataProviderId;
        const ramses::dataConsumerId_t m_dataConsumerId;

        const ramses::sceneId_t m_sceneProviderId;
        const ramses::sceneId_t m_sceneConsumerId;
        ramses_internal::ClientScene m_sceneProvider;
        ramses_internal::ClientScene m_sceneConsumer;

        ramses::displayId_t displayId;
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unsubscribeScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneUnsubscribe)
    {
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unsubscribeScene(m_sceneId));
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
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedScene(m_sceneId, m_scene);
        mapScene(m_sceneId, displayId);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForMapOfUnknownScene)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        mapScene(m_sceneId, displayId, false);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneMapOnInvalidDisplay)
    {
        const ramses::displayId_t displayId(1);
        mapScene(m_sceneId, displayId, false);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneUnmap)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();

        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        mapScene(m_sceneId, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneUnmap)
    {
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesINDIRECTEventForSceneUnmapAfterSceneUnpublished)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();

        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        mapScene(m_sceneId, displayId);

        unpublishScene();
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectSceneUnpublished(m_sceneId);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneShow)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneShow)
    {
        createPublishedAndSubscribedScene(m_sceneId, m_scene);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesOKEventForSceneHide)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.showScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneShown(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.hideScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneHidden(m_sceneId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneHide)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.hideScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneHidden(m_sceneId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesINDIRECTEventForSceneHideAfterSceneUnpublished)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.showScene(m_sceneId));
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
        ON_CALL(*gPlatformFactoryMock, createRenderBackend(_, _)).WillByDefault(Return(static_cast<ramses_internal::IRenderBackend*>(nullptr)));
        createDisplayAndExpectResult(ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForDisplayDestruction)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForInvalidDisplayDestruction)
    {
        const ramses::displayId_t displayId(1u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));

        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForDisplayDestructionIfAnySceneMappedToIt)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForDisplayDestructionAfterFailedAttemptDueToMappedScene)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_FAIL);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unmapScene(m_sceneId));
        updateAndDispatch(m_handler);
        m_handler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(displayId));
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(displayId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWarpingMeshDataUpdate)
    {
        ramses::displayId_t displayId = addDisplay(true);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(displayId, ramses::ERendererEventResult_OK);
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
        ramses::displayId_t displayId = addDisplay(false);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const uint16_t indices[3] = { 0 };
        const float vertices[9] = { 0.f };
        const ramses::WarpingMeshData warpData(3, indices, 3, vertices, vertices);
        EXPECT_EQ(ramses::StatusOK, m_renderer.updateWarpingMeshData(displayId, warpData));

        updateAndDispatch(m_handler);

        m_handler.expectWarpingMeshDataUpdated(displayId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferCreation)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferCreation)
    {
        ramses::displayId_t displayId(0u);
        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t displayId = addDisplay();
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForOffscreenBufferDestruction)
    {
        ramses::displayId_t displayId(0u);
        const ramses::displayBufferId_t bufferId(0u);
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyOffscreenBuffer(displayId, bufferId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferDestroyed(displayId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneAssignedToOffscreenBuffer)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(m_sceneId, bufferId, 11));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToDisplayBuffer(m_sceneId, bufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneAssignedToOffscreenBuffer)
    {
        const ramses::displayBufferId_t bufferId(2u);
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(m_sceneId, bufferId, 11));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToDisplayBuffer(m_sceneId, bufferId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForSceneAssignedToFramebuffer)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const auto displayBufferId = m_renderer.getDisplayFramebuffer(displayId);
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(m_sceneId, displayBufferId, 11));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToDisplayBuffer(m_sceneId, displayBufferId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatch, generatesFAILEventForSceneAssignedToFramebuffer)
    {
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.assignSceneToDisplayBuffer(m_sceneId, ramses::displayBufferId_t::Invalid(), 11));
        updateAndDispatch(m_handler);
        m_handler.expectSceneAssignedToDisplayBuffer(m_sceneId, ramses::displayBufferId_t::Invalid(), ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowClosed)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onClose();
        updateAndDispatch(m_handler);
        m_handler.expectWindowClosed(displayId);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowResized)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onResize(100, 200);
        updateAndDispatch(m_handler);
        m_handler.expectWindowResized(displayId, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForWindowMoved)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onWindowMove(100, 200);
        updateAndDispatch(m_handler);
        m_handler.expectWindowMoved(displayId, 100, 200);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyPressed)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Pressed,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_E);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(displayId, ramses::EKeyEvent_Pressed, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_E);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForKeyReleased)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
        m_renderer.impl.getRenderer().getRenderer().getDisplayEventHandler(displayHandle).onKeyEvent(ramses_internal::EKeyEventType_Released,
            ramses_internal::EKeyModifier_Ctrl | ramses_internal::EKeyModifier_Shift, ramses_internal::EKeyCode_F);
        updateAndDispatch(m_handler);
        m_handler.expectKeyEvent(displayId, ramses::EKeyEvent_Released, ramses::EKeyModifier_Ctrl | ramses::EKeyModifier_Shift, ramses::EKeyCode_F);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventsForMouseActionsOnDisplay)
    {
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        const ramses_internal::DisplayHandle displayHandle(displayId.getValue());
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
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_RightButtonDown, 20, 30);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_RightButtonUp, 21, 31);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_LeftButtonDown, 22, 32);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_LeftButtonUp, 23, 33);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_MiddleButtonDown, 24, 34);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_MiddleButtonUp, 25, 35);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WheelDown, 26, 36);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WheelUp, 27, 37);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_Move, 28, 38);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WindowEnter, 29, 39);
        m_handler.expectMouseEvent(displayId, ramses::EMouseEvent_WindowLeave, 30, 40);
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForObjectsPicked)
    {
        EXPECT_CALL(gPlatformFactoryMock->renderBackendMock.surfaceMock.windowMock, getWidth()).WillRepeatedly(Return(1280));
        EXPECT_CALL(gPlatformFactoryMock->renderBackendMock.surfaceMock.windowMock, getHeight()).WillRepeatedly(Return(480));
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        //create pickable and add to mesh
        SceneActionCollection sceneActions;
        SceneActionCollectionCreator creator(sceneActions);
        const NodeHandle            nodeHandle{ 1u };
        const TransformHandle       transformHandle{ 2u };
        const DataBufferHandle      geometryDataBufferHandle{ 3u };
        const ramses::pickableObjectId_t pickableId{ 4u };
        const PickableObjectHandle  pickableHandle{ 5u };
        const DataLayoutHandle      viewportDataLayout{ 5u };
        const DataLayoutHandle      viewportDataReferenceLayout{ 6u };
        const DataInstanceHandle    viewportDataInstance{ 6u };
        const DataInstanceHandle    viewportOffsetDataReference{ 7u };
        const DataInstanceHandle    viewportSizeDataReference{ 8u };
        const CameraHandle          cameraHandle{ 7u };

        const Frustum frustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
        const std::array<float, 9> geometryData{ -1.f, 0.f, -0.5f, 0.f, 1.f, -0.5f, 0.f, 0.f, -0.5f };

        creator.allocateNode(1, nodeHandle);
        creator.allocateTransform(nodeHandle, transformHandle);
        creator.allocateDataBuffer(EDataBufferType::VertexBuffer, EDataType_Vector3F, UInt32(geometryData.size() * sizeof(float)), geometryDataBufferHandle);
        creator.updateDataBuffer(geometryDataBufferHandle, 0, UInt32(geometryData.size() * sizeof(float)), reinterpret_cast<const Byte*>(geometryData.data()));

        creator.allocateDataLayout({ {EDataType_DataReference}, {EDataType_DataReference} }, ResourceContentHash::Invalid(), viewportDataLayout);
        creator.allocateDataInstance(viewportDataLayout, viewportDataInstance);
        creator.allocateDataLayout({ { EDataType_Vector2I } }, ResourceContentHash::Invalid(), viewportDataReferenceLayout);
        creator.allocateDataInstance(viewportDataReferenceLayout, viewportOffsetDataReference);
        creator.allocateDataInstance(viewportDataReferenceLayout, viewportSizeDataReference);
        creator.setDataReference(viewportDataInstance, Camera::ViewportOffsetField, viewportOffsetDataReference);
        creator.setDataReference(viewportDataInstance, Camera::ViewportSizeField, viewportSizeDataReference);
        const Vector2i viewportOffset{ 0, 0 };
        const Vector2i viewportSize{ 1280, 480 };
        creator.setDataVector2iArray(viewportOffsetDataReference, ramses_internal::DataFieldHandle{ 0 }, 1u, &viewportOffset);
        creator.setDataVector2iArray(viewportSizeDataReference, ramses_internal::DataFieldHandle{ 0 }, 1u, &viewportSize);
        creator.allocateCamera(ECameraProjectionType_Orthographic, nodeHandle, viewportDataInstance, cameraHandle);

        creator.setCameraFrustum(cameraHandle, frustum);
        creator.allocatePickableObject(geometryDataBufferHandle, nodeHandle, PickableObjectId(pickableId.getValue()), pickableHandle);
        creator.setPickableObjectCamera(pickableHandle, cameraHandle);

        creator.flush(1u, true, ramses_internal::SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(m_sceneId.getValue()), EScenePublicationMode_LocalOnly);
        updateAndDispatch(m_handler);

        m_renderer.impl.handlePickEvent(m_sceneId, -0.375000f, 0.250000f);
        updateAndDispatch(m_handler);
        m_handler.expectObjectsPicked(m_sceneId, { pickableId });
    }

    TEST_F(ARamsesRendererDispatch, generatesEventForObjectsPicked_regressionRoundingIssues)
    {
        EXPECT_CALL(gPlatformFactoryMock->renderBackendMock.surfaceMock.windowMock, getWidth()).WillRepeatedly(Return(1280));
        EXPECT_CALL(gPlatformFactoryMock->renderBackendMock.surfaceMock.windowMock, getHeight()).WillRepeatedly(Return(480));
        const ramses::displayId_t displayId = createDisplayAndExpectResult();
        createPublishedAndSubscribedAndMappedScene(m_sceneId, m_scene, displayId);

        //create two pickables and add to mesh
        SceneActionCollection sceneActions;
        SceneActionCollectionCreator creator(sceneActions);
        const NodeHandle            cameraNodeHandle{ 1u };
        const NodeHandle            pickable1NodeHandle{ 2u };
        const NodeHandle            pickable2NodeHandle{ 3u };
        const TransformHandle       cameraTransformHandle{ 1u };
        const TransformHandle       pickable1TransformHandle{ 2u };
        const TransformHandle       pickable2TransformHandle{ 3u };
        const DataBufferHandle      geometryDataBufferHandle{ 3u };
        const ramses::pickableObjectId_t pickable1Id{ 4u };
        const ramses::pickableObjectId_t pickable2Id{ 5u };
        const PickableObjectHandle  pickable1Handle{ 5u };
        const PickableObjectHandle  pickable2Handle{ 6u };
        const DataLayoutHandle      viewportDataLayout{ 5u };
        const DataLayoutHandle      viewportDataReferenceLayout{ 6u };
        const DataInstanceHandle    viewportDataInstance{ 6u };
        const DataInstanceHandle    viewportOffsetDataReference{ 7u };
        const DataInstanceHandle    viewportSizeDataReference{ 8u };
        const CameraHandle          cameraHandle{ 7u };

        const ProjectionParams params = ramses_internal::ProjectionParams::Perspective(19.f, 1280.f / 480.f, 0.1f, 100.f);
        const Frustum frustum(params.leftPlane, params.rightPlane, params.bottomPlane, params.topPlane, params.nearPlane, params.farPlane);

        //nodes
        creator.allocateNode(1, cameraNodeHandle);
        creator.allocateNode(1, pickable1NodeHandle);
        creator.allocateNode(1, pickable2NodeHandle);

        //transforms
        creator.allocateTransform(cameraNodeHandle, cameraTransformHandle);
        creator.allocateTransform(pickable1NodeHandle, pickable1TransformHandle);
        creator.allocateTransform(pickable2NodeHandle, pickable2TransformHandle);

        creator.setTransformComponent(ETransformPropertyType_Translation, cameraTransformHandle, { -4.f, 0.f, 11.f });
        creator.setTransformComponent(ETransformPropertyType_Rotation, cameraTransformHandle, { 0.f, 40.f, 0.f });

        creator.setTransformComponent(ETransformPropertyType_Translation, pickable1TransformHandle, { 0.1f, 1.0f, -1.0f });
        creator.setTransformComponent(ETransformPropertyType_Rotation, pickable1TransformHandle, { 70.0f, 0.0f, 0.0f });
        creator.setTransformComponent(ETransformPropertyType_Scaling, pickable1TransformHandle, { 3.0f, 3.0f, 3.0f });

        creator.setTransformComponent(ETransformPropertyType_Translation, pickable2TransformHandle, { 6.0f, 0.9f, -1.0f });
        creator.setTransformComponent(ETransformPropertyType_Rotation, pickable2TransformHandle, { 70.0f, 0.0f, 0.0f });
        creator.setTransformComponent(ETransformPropertyType_Scaling, pickable2TransformHandle, { 3.0f, 3.0f, 3.0f });

        //geometry
        const std::array<float, 9> geometryData{ -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
        creator.allocateDataBuffer(EDataBufferType::VertexBuffer, EDataType_Vector3F, UInt32(geometryData.size() * sizeof(float)), geometryDataBufferHandle);
        creator.updateDataBuffer(geometryDataBufferHandle, 0, UInt32(geometryData.size() * sizeof(float)), reinterpret_cast<const Byte*>(geometryData.data()));

        //camera
        creator.allocateDataLayout({ {EDataType_DataReference}, {EDataType_DataReference} }, ResourceContentHash::Invalid(), viewportDataLayout);
        creator.allocateDataInstance(viewportDataLayout, viewportDataInstance);
        creator.allocateDataLayout({ { EDataType_Vector2I } }, ResourceContentHash::Invalid(), viewportDataReferenceLayout);
        creator.allocateDataInstance(viewportDataReferenceLayout, viewportOffsetDataReference);
        creator.allocateDataInstance(viewportDataReferenceLayout, viewportSizeDataReference);
        creator.setDataReference(viewportDataInstance, Camera::ViewportOffsetField, viewportOffsetDataReference);
        creator.setDataReference(viewportDataInstance, Camera::ViewportSizeField, viewportSizeDataReference);
        const Vector2i viewportOffset{ 0, 0 };
        const Vector2i viewportSize{ 1280, 480 };
        creator.setDataVector2iArray(viewportOffsetDataReference, ramses_internal::DataFieldHandle{ 0 }, 1u, &viewportOffset);
        creator.setDataVector2iArray(viewportSizeDataReference, ramses_internal::DataFieldHandle{ 0 }, 1u, &viewportSize);
        creator.allocateCamera(ECameraProjectionType_Perspective, cameraNodeHandle, viewportDataInstance, cameraHandle);
        creator.setCameraFrustum(cameraHandle, frustum);

        //pickables
        creator.allocatePickableObject(geometryDataBufferHandle, pickable1NodeHandle, PickableObjectId(pickable1Id.getValue()), pickable1Handle);
        creator.setPickableObjectCamera(pickable1Handle, cameraHandle);
        creator.allocatePickableObject(geometryDataBufferHandle, pickable2NodeHandle, PickableObjectId(pickable2Id.getValue()), pickable2Handle);
        creator.setPickableObjectCamera(pickable2Handle, cameraHandle);

        creator.flush(1u, true, ramses_internal::SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(sceneActions), ramses_internal::SceneId(m_sceneId.getValue()), EScenePublicationMode_LocalOnly);
        updateAndDispatch(m_handler);

        m_renderer.impl.handlePickEvent(m_sceneId, -0.382812f, 0.441667f);
        updateAndDispatch(m_handler);
        m_handler.expectObjectsPicked(m_sceneId, { pickable1Id });

        m_renderer.impl.handlePickEvent(m_sceneId, -0.379687f, 0.395833f);
        updateAndDispatch(m_handler);
        m_handler.expectObjectsPicked(m_sceneId, { pickable2Id });
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForLinkWithWrongParameters)
    {
        const ramses::sceneId_t invalidSceneProviderId(777u);
        const ramses::sceneId_t invalidSceneConsumerId(778u);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(invalidSceneProviderId, m_dataProviderId, invalidSceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unlinkData(m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForUnlinkWithWrongParameters)
    {
        const ramses::sceneId_t invalidSceneConsumerId(778u);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unlinkData(invalidSceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unlinkData(m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unlinkData(m_sceneConsumerId, m_dataConsumerId));
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

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectDataLinked(m_sceneProviderId, m_dataProviderId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);

        destroyDataSlot(m_sceneConsumerId, m_dataConsumerHandle);

        updateAndDispatch(m_handler);
        m_handler.expectDataUnlinked(m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_INDIRECT);
        m_handler.expectDataConsumerDestroyed(m_sceneConsumerId, m_dataConsumerId);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesOKEventForOffscreenBufferLink)
    {
        const ramses::displayBufferId_t bufferId = m_renderer.createOffscreenBuffer(displayId, 1u, 1u);
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferCreated(displayId, bufferId, ramses::ERendererEventResult_OK);

        createTextureSlotConsumer(m_sceneConsumerId);
        updateAndDispatch(m_handler);
        m_handler.expectDataConsumerCreated(m_sceneConsumerId, m_dataConsumerId);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkOffscreenBufferToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferLinkedToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_OK);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, generatesFAILEventForOffscreenBufferLink)
    {
        const ramses::displayBufferId_t bufferId(1u);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkOffscreenBufferToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId));
        updateAndDispatch(m_handler);
        m_handler.expectOffscreenBufferLinkedToSceneData(bufferId, m_sceneConsumerId, m_dataConsumerId, ramses::ERendererEventResult_FAIL);
    }

    TEST_F(ARamsesRendererDispatchWithProviderConsumerScenes, confidenceTest_generatesAppropriateEventsForFullCycleOfSceneWithMapAndDataLink_commandsBatched)
    {
        const ramses::sceneId_t customSceneProviderId(777u);
        const ramses::sceneId_t customsceneConsumerId(778u);
        const ramses_internal::SceneId sceneProviderIdInternal(customSceneProviderId.getValue());
        const ramses_internal::SceneId sceneConsumerIdInternal(customsceneConsumerId.getValue());
        const SceneInfo createInfoProvider(sceneProviderIdInternal);
        const SceneInfo createInfoConsumer(sceneConsumerIdInternal);
        ramses_internal::ClientScene sceneProvider(createInfoProvider);
        ramses_internal::ClientScene sceneConsumer(createInfoConsumer);

        // publish scenes
        m_sceneGraphProvider.handleCreateScene(sceneProvider, false, m_eventConsumer);
        m_sceneGraphProvider.handleCreateScene(sceneConsumer, false, m_eventConsumer);
        m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(customSceneProviderId.getValue()), EScenePublicationMode_LocalOnly);
        m_sceneGraphProvider.handlePublishScene(ramses_internal::SceneId(customsceneConsumerId.getValue()), EScenePublicationMode_LocalOnly);
        updateAndDispatch(m_handler);
        m_handler.expectScenePublished(customSceneProviderId);
        m_handler.expectScenePublished(customsceneConsumerId);

        // subscribe and emulate receive scenes
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.subscribeScene(customSceneProviderId));
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.subscribeScene(customsceneConsumerId));
        updateAndDispatch(m_handler); // needed to process subscription request
        m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(customSceneProviderId.getValue())), EScenePublicationMode_LocalOnly);
        m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(customsceneConsumerId.getValue())), EScenePublicationMode_LocalOnly);

        // create data slots
        const ramses_internal::NodeHandle node(7u);
        const ramses_internal::DataInstanceHandle dataRef;
        SceneActionCollection sceneActions;
        SceneActionCollectionCreator creator(sceneActions);

        creator.allocateNode(0u, node);
        creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationProvider, ramses_internal::DataSlotId(m_dataProviderId.getValue()), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataProviderHandle);
        creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, sceneActions.copy(), ramses_internal::SceneId(customSceneProviderId.getValue()), EScenePublicationMode_LocalOnly);
        sceneActions.clear();

        creator.allocateNode(0u, node);
        creator.allocateDataSlot({ ramses_internal::EDataSlotType_TransformationConsumer, ramses_internal::DataSlotId(m_dataConsumerId.getValue()), node, dataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, m_dataConsumerHandle);
        creator.flush(1u, true, SceneSizeInformation(10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u, 10u));
        m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, sceneActions.copy(), ramses_internal::SceneId(customsceneConsumerId.getValue()), EScenePublicationMode_LocalOnly);

        // create data link
        m_renderer.flush();
        doUpdateLoop();
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.linkData(customSceneProviderId, m_dataProviderId, customsceneConsumerId, m_dataConsumerId));

        // create display
        const ramses::displayId_t display = addDisplay();

        // map scenes
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.mapScene(display, customSceneProviderId));
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.mapScene(display, customsceneConsumerId));

        // cleanup
        m_renderer.flush();
        m_sceneControlAPI.flush(); // emulation of client commands via sceneGraphProvider go directly to m_renderer command buffer, to keep order of commands previous m_renderer API calls must be flushed
        doUpdateLoop(); // needed for successfully processing mapped commands
        doUpdateLoop(); // needed for successfully processing mapped commands

        m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(customSceneProviderId.getValue()));
        m_sceneGraphProvider.handleUnpublishScene(ramses_internal::SceneId(customsceneConsumerId.getValue()));
        EXPECT_EQ(ramses::StatusOK, m_renderer.destroyDisplay(display));

        // process commands
        updateAndDispatch(m_handler);
        m_handler.expectDisplayDestroyed(display, ramses::ERendererEventResult_OK);
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
                m_renderer.getSceneControlAPI_legacy()->flush();
                m_renderer.impl.getRenderer().doOneLoop(ramses_internal::ELoopMode::UpdateOnly);
                m_renderer.dispatchEvents(*this);
                m_renderer.getSceneControlAPI_legacy()->dispatchEvents(*this);
            }

            virtual void scenePublished(ramses::sceneId_t scnId) override
            {
                RendererEventTestHandler::scenePublished(scnId);
                if (scnId == m_sceneId)
                {
                    EXPECT_EQ(ramses::StatusOK, m_renderer.getSceneControlAPI_legacy()->subscribeScene(m_sceneId));
                    updateAndDispatch();
                    // receive m_scene
                    m_sceneGraphSender.sendCreateScene(m_framework.impl.getParticipantAddress().getParticipantId(), SceneInfo(ramses_internal::SceneId(m_sceneId.getValue())), EScenePublicationMode_LocalOnly);
                    // receive initial flush
                    SceneActionCollection initialSceneActions;
                    SceneActionCollectionCreator creator(initialSceneActions);
                    creator.flush(1u, false);
                    m_sceneGraphSender.sendSceneActionList({ m_framework.impl.getParticipantAddress().getParticipantId() }, std::move(initialSceneActions), ramses_internal::SceneId(m_sceneId.getValue()), EScenePublicationMode_LocalOnly);
                    updateAndDispatch();
                }
            }

            virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
            {
                RendererEventTestHandler::sceneSubscribed(sceneId, result);
                if (sceneId == m_sceneId && result == ramses::ERendererEventResult_OK)
                {
                    EXPECT_EQ(ramses::StatusOK, m_renderer.getSceneControlAPI_legacy()->mapScene(m_displayId, m_sceneId));
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
        ramses::displayId_t displayId = addDisplay(false);
        updateAndDispatch(m_handler);
        m_handler.expectDisplayCreated(displayId, ramses::ERendererEventResult_OK);

        //m_scene publishing should result in mapped Scene after dispatching events to the recursive m_handler
        RecursiveRendererEventTestHandler recHandler(m_renderer, m_sceneId, displayId, m_sceneGraphSender, m_framework);
        publishScene();
        updateAndDispatch(recHandler);
        recHandler.expectScenePublished(m_sceneId);
        recHandler.expectSceneSubscribed(m_sceneId, ramses::ERendererEventResult_OK);
        recHandler.expectSceneMapped(m_sceneId, ramses::ERendererEventResult_OK);

        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unmapScene(m_sceneId));
        EXPECT_EQ(ramses::StatusOK, m_sceneControlAPI.unsubscribeScene(m_sceneId));
        updateAndDispatch(recHandler);
        recHandler.expectSceneUnmapped(m_sceneId, ramses::ERendererEventResult_OK);
        recHandler.expectSceneUnsubscribed(m_sceneId, ramses::ERendererEventResult_OK);
    }

}
