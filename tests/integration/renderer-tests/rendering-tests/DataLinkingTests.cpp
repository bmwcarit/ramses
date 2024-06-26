//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DataLinkingTests.h"
#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/MultiTransformationLinkScene.h"
#include "TestScenes/DataLinkScene.h"
#include "TestScenes/CameraDataLinkScene.h"
#include "TestScenes/TextureLinkScene.h"
#include "TestScenes/MultiTypeLinkScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "ramses/client/Node.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Texture2D.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    void DataLinkingTests::setUpTestCases(RendererTestsFramework& testFramework)
    {
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_ConsumerNotLinkedToProvider, *this, "TransformationLinkTest_ConsumerNotLinkedToProvider");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_ConsumerLinkedToProvider, *this, "TransformationLinkTest_ConsumerLinkedToProvider");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_LinkRemoved, *this, "TransformationLinkTest_LinkRemoved");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_LinkOverridesComsumerTransform, *this, "TransformationLinkTest_LinkOverridesComsumerTransform");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_ConsumerLinkedToProviderNested, *this, "TransformationLinkTest_ConsumerLinkedToProviderNested");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_RemovedProviderFromScene, *this, "TransformationLinkTest_RemovedProviderFromScene");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_RemovedConsumerFromScene, *this, "TransformationLinkTest_RemovedConsumerFromScene");
        testFramework.createTestCaseWithDefaultDisplay(TransformationLinkTest_ConfidenceMultiLinkTest, *this, "TransformationLinkTest_ConfidenceMultiLinkTest");

        testFramework.createTestCaseWithDefaultDisplay(DataLinkTest_NoLinks, *this, "DataLinkTest_NoLinks");
        testFramework.createTestCaseWithDefaultDisplay(DataLinkTest_Linked, *this, "DataLinkTest_Linked");
        testFramework.createTestCaseWithDefaultDisplay(DataLinkTest_LinksRemoved, *this, "DataLinkTest_LinksRemoved");
        testFramework.createTestCaseWithDefaultDisplay(DataLinkTest_ProviderRemoved, *this, "DataLinkTest_ProviderRemoved");
        testFramework.createTestCaseWithDefaultDisplay(DataLinkTest_ConsumerRemoved, *this, "DataLinkTest_ConsumerRemoved");

        testFramework.createTestCaseWithDefaultDisplay(CameraDataLinkTest_NoLinks, *this, "CameraDataLinkTest_NoLinks");
        testFramework.createTestCaseWithDefaultDisplay(CameraDataLinkTest_ViewportLinked, *this, "CameraDataLinkTest_ViewportLinked");
        testFramework.createTestCaseWithDefaultDisplay(CameraDataLinkTest_FrustumLinked, *this, "CameraDataLinkTest_FrustumLinked");

        testFramework.createTestCaseWithDefaultDisplay(TextureLinkTest_NoLinks, *this, "TextureLinkTest_NoLinks");
        testFramework.createTestCaseWithDefaultDisplay(TextureLinkTest_Linked, *this, "TextureLinkTest_Linked");
        testFramework.createTestCaseWithDefaultDisplay(TextureLinkTest_LinksRemoved, *this, "TextureLinkTest_LinksRemoved");
        testFramework.createTestCaseWithDefaultDisplay(TextureLinkTest_ProviderSceneUnmapped, *this, "TextureLinkTest_ProviderSceneUnmapped");
        // TODO(Carsten): Find a solution for this test with Vaclav
        //testFramework.createTestCaseWithDefaultDisplay(TextureLinkTest_ProvidedTextureChanges, *this, "TextureLinkTest_ProvidedTextureChanges");

        testFramework.createTestCaseWithDefaultDisplay(MultiTypeLinkTest_NoLinks, *this, "MultiTypeLinkTest_NoLinks");
        testFramework.createTestCaseWithDefaultDisplay(MultiTypeLinkTest_Linked, *this, "MultiTypeLinkTest_Linked");
    }

    bool DataLinkingTests::run(RendererTestsFramework& testFramework, const RenderingTestCase& testCase)
    {
        std::string expectedImageName("!!unknown!!");
        float expectedPixelError = RendererTestUtils::DefaultMaxAveragePercentPerPixel;
        switch (testCase.m_id)
        {
        case TransformationLinkTest_ConsumerNotLinkedToProvider:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerOvrSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER_OVERRIDEN,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consAndProvSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER_AND_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consumerSceneId);
            testFramework.publishAndFlushScene(consumerOvrSceneId);
            testFramework.publishAndFlushScene(consAndProvSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consumerSceneId);
            testFramework.getSceneToRendered(consumerOvrSceneId);
            testFramework.getSceneToRendered(consAndProvSceneId);
            expectedImageName = "DataLinkTest_AllLinksDisabled";
            break;
        }
        case TransformationLinkTest_ConsumerLinkedToProvider:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consumerSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consumerSceneId);
            testFramework.createDataLink(providerSceneId, TransformationLinkScene::transformProviderDataId, consumerSceneId, TransformationLinkScene::transformConsumerDataId);
            expectedImageName = "DataLinkTest_ConsumerLinkedToProvider";
            break;
        }
        case TransformationLinkTest_LinkOverridesComsumerTransform:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerOvrSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER_OVERRIDEN,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consumerOvrSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consumerOvrSceneId);
            testFramework.createDataLink(providerSceneId, TransformationLinkScene::transformProviderDataId, consumerOvrSceneId, TransformationLinkScene::transformConsumerDataId);
            expectedImageName = "DataLinkTest_LinkOverridesComsumerTransform";
            break;
        }
        case TransformationLinkTest_LinkRemoved:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consumerSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consumerSceneId);
            testFramework.createDataLink(providerSceneId, TransformationLinkScene::transformProviderDataId, consumerSceneId, TransformationLinkScene::transformConsumerDataId);
            testFramework.removeDataLink(consumerSceneId, TransformationLinkScene::transformConsumerDataId);
            expectedImageName = "DataLinkTest_LinkRemoved";
            break;
        }
        case TransformationLinkTest_ConsumerLinkedToProviderNested:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consAndProvSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER_AND_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consAndProvSceneId);
            testFramework.publishAndFlushScene(consumerSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consAndProvSceneId);
            testFramework.getSceneToRendered(consumerSceneId);
            testFramework.createDataLink(providerSceneId, TransformationLinkScene::transformProviderDataId, consAndProvSceneId, TransformationLinkScene::transformConsumerDataId);
            testFramework.createDataLink(consAndProvSceneId, TransformationLinkScene::transformProviderDataId, consumerSceneId, TransformationLinkScene::transformConsumerDataId);
            expectedImageName = "DataLinkTest_ConsumerLinkedToProviderNested";
            break;
        }
        case TransformationLinkTest_RemovedProviderFromScene:
        case TransformationLinkTest_RemovedConsumerFromScene:
        {
            const ramses::sceneId_t providerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            const ramses::sceneId_t consumerSceneId = testFramework.getScenesRegistry().createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER,
                glm::vec3(0.0f, 0.0f, 12.0f));
            testFramework.publishAndFlushScene(providerSceneId);
            testFramework.publishAndFlushScene(consumerSceneId);
            testFramework.getSceneToRendered(providerSceneId);
            testFramework.getSceneToRendered(consumerSceneId);
            testFramework.createDataLink(providerSceneId, TransformationLinkScene::transformProviderDataId, consumerSceneId, TransformationLinkScene::transformConsumerDataId);

            if (TransformationLinkTest_RemovedProviderFromScene == testCase.m_id)
            {
                ramses::Scene& scene = testFramework.getScenesRegistry().getScene(providerSceneId);
                ramses::RamsesObject* provider = scene.findObject("transform provider");
                assert(provider != nullptr);

                scene.destroy(RamsesObjectTypeUtils::ConvertTo<ramses::SceneObject>(*provider));
                scene.flush();
            }
            else if (TransformationLinkTest_RemovedConsumerFromScene == testCase.m_id)
            {
                ramses::Scene& scene = testFramework.getScenesRegistry().getScene(consumerSceneId);
                ramses::RamsesObject* consumer = scene.findObject("transform consumer");
                assert(consumer != nullptr);

                scene.destroy(RamsesObjectTypeUtils::ConvertTo<ramses::SceneObject>(*consumer));
                scene.flush();
            }

            expectedImageName = "DataLinkTest_LinkRemoved";
            break;
        }
        case TransformationLinkTest_ConfidenceMultiLinkTest:
        {
            m_sceneIdProvider         = testFramework.getScenesRegistry().createScene<MultiTransformationLinkScene>(MultiTransformationLinkScene::PROVIDER_SCENE);
            m_sceneIdProviderConsumer = testFramework.getScenesRegistry().createScene<MultiTransformationLinkScene>(MultiTransformationLinkScene::PROVIDER_CONSUMER_SCENE);
            m_sceneIdConsumer         = testFramework.getScenesRegistry().createScene<MultiTransformationLinkScene>(MultiTransformationLinkScene::CONSUMER_SCENE);

            testFramework.publishAndFlushScene(m_sceneIdProvider);
            testFramework.publishAndFlushScene(m_sceneIdProviderConsumer);
            testFramework.publishAndFlushScene(m_sceneIdConsumer);
            testFramework.getSceneToRendered(m_sceneIdProvider);
            testFramework.getSceneToRendered(m_sceneIdProviderConsumer);
            testFramework.getSceneToRendered(m_sceneIdConsumer);

            for (uint32_t rowId = MultiTransformationLinkScene::DataIdRowStart; rowId < MultiTransformationLinkScene::DataIdRowStart + 4u; ++rowId)
            {
                testFramework.createDataLink(m_sceneIdProvider, ramses::dataProviderId_t{rowId}, m_sceneIdProviderConsumer, ramses::dataConsumerId_t{rowId});
            }

            for (uint32_t meshId = MultiTransformationLinkScene::DataIdMeshStart; meshId < MultiTransformationLinkScene::DataIdMeshStart + 16u; ++meshId)
            {
                testFramework.createDataLink(m_sceneIdProviderConsumer, ramses::dataProviderId_t{meshId}, m_sceneIdConsumer, ramses::dataConsumerId_t{meshId});
            }

            expectedImageName = "DataLinkTest_ConfidenceMultiLink";
            break;
        }

        case DataLinkTest_NoLinks:
            createAndShowDataLinkScenes<DataLinkScene>(testFramework);
            expectedImageName = "DataLinkTest_NoLinks";
            break;
        case DataLinkTest_Linked:
            createAndShowDataLinkScenes<DataLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, DataLinkScene::DataProviderId, m_sceneIdProviderConsumer, DataLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, DataLinkScene::DataProviderId, m_sceneIdConsumer, DataLinkScene::DataConsumerId);
            expectedImageName = "DataLinkTest_Linked";
            break;
        case DataLinkTest_LinksRemoved:
            createAndShowDataLinkScenes<DataLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, DataLinkScene::DataProviderId, m_sceneIdProviderConsumer, DataLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, DataLinkScene::DataProviderId, m_sceneIdConsumer, DataLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("DataLinkTest_Linked", 0u, 0.0f))
            {
                return false;
            }
            testFramework.removeDataLink(m_sceneIdConsumer, DataLinkScene::DataConsumerId);
            testFramework.removeDataLink(m_sceneIdProviderConsumer, DataLinkScene::DataConsumerId);
            expectedImageName = "DataLinkTest_NoLinks";
            break;
        case DataLinkTest_ProviderRemoved:
        {
            createAndShowDataLinkScenes<DataLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, DataLinkScene::DataProviderId, m_sceneIdProviderConsumer, DataLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, DataLinkScene::DataProviderId, m_sceneIdConsumer, DataLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("DataLinkTest_Linked", 0u, 0.0f))
            {
                return false;
            }

            ramses::Scene& scene = testFramework.getScenesRegistry().getScene(m_sceneIdProvider);
            auto& consumerAppearance1 = *scene.findObject<Appearance>("dataLinkAppearance1");
            auto& consumerAppearance2 = *scene.findObject<Appearance>("dataLinkAppearance2");
            const auto colorInput = consumerAppearance1.getEffect().findUniformInput("color");
            consumerAppearance1.unbindInput(*colorInput);
            consumerAppearance2.unbindInput(*colorInput);

            auto& colorData = *scene.findObject<SceneObject>("dataLinkColorData");
            scene.destroy(colorData);
            scene.flush();

            expectedImageName = "DataLinkTest_RemovedProvider";
        }
            break;
        case DataLinkTest_ConsumerRemoved:
        {
            createAndShowDataLinkScenes<DataLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, DataLinkScene::DataProviderId, m_sceneIdProviderConsumer, DataLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, DataLinkScene::DataProviderId, m_sceneIdConsumer, DataLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("DataLinkTest_Linked", 0u, 0.0f))
            {
                return false;
            }

            ramses::Scene& scene = testFramework.getScenesRegistry().getScene(m_sceneIdConsumer);
            auto& consumerAppearance1 = *scene.findObject<Appearance>("dataLinkAppearance1");
            auto& consumerAppearance2 = *scene.findObject<Appearance>("dataLinkAppearance2");
            const auto colorInput = consumerAppearance1.getEffect().findUniformInput("color");
            consumerAppearance1.unbindInput(*colorInput);
            consumerAppearance2.unbindInput(*colorInput);

            auto& colorData = *scene.findObject<SceneObject>("dataLinkColorData");
            scene.destroy(colorData);
            scene.flush();

            expectedImageName = "DataLinkTest_RemovedConsumer";
        }
            break;

        case CameraDataLinkTest_NoLinks:
            testFramework.createAndShowScene<CameraDataLinkScene>(CameraDataLinkScene::CAMERADATA_CONSUMER, m_cameraMid);
            expectedImageName = "CameraData_NoLinks";
            break;
        case CameraDataLinkTest_ViewportLinked:
        {
            const auto providerSceneId = testFramework.createAndShowScene<CameraDataLinkScene>(CameraDataLinkScene::CAMERADATA_PROVIDER, m_cameraMid);
            const auto consumerSceneId = testFramework.createAndShowScene<CameraDataLinkScene>(CameraDataLinkScene::CAMERADATA_CONSUMER, m_cameraMid);
            testFramework.createDataLink(providerSceneId, CameraDataLinkScene::ViewportOffsetProviderId, consumerSceneId, CameraDataLinkScene::ViewportOffsetConsumerId);
            testFramework.createDataLink(providerSceneId, CameraDataLinkScene::ViewportSizeProviderId, consumerSceneId, CameraDataLinkScene::ViewportSizeConsumerId);
            expectedImageName = "CameraData_ViewportLinked";
        }
            break;
        case CameraDataLinkTest_FrustumLinked:
        {
            const auto providerSceneId = testFramework.createAndShowScene<CameraDataLinkScene>(CameraDataLinkScene::CAMERADATA_PROVIDER, m_cameraMid);
            const auto consumerSceneId = testFramework.createAndShowScene<CameraDataLinkScene>(CameraDataLinkScene::CAMERADATA_CONSUMER, m_cameraMid);
            testFramework.createDataLink(providerSceneId, CameraDataLinkScene::FrustumPlanesProviderId, consumerSceneId, CameraDataLinkScene::FrustumPlanesConsumerId);
            testFramework.createDataLink(providerSceneId, CameraDataLinkScene::FrustumPlanesNearFarProviderId, consumerSceneId, CameraDataLinkScene::FrustumPlanesNearFarConsumerId);
            expectedImageName = "CameraData_FrustumLinked";
        }
        break;

        case TextureLinkTest_NoLinks:
            createAndShowDataLinkScenes<TextureLinkScene>(testFramework);
            expectedImageName = "TextureLinkTest_NoLinks";
            break;
        case TextureLinkTest_Linked:
            createAndShowDataLinkScenes<TextureLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, TextureLinkScene::DataProviderId, m_sceneIdProviderConsumer, TextureLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, TextureLinkScene::DataProviderId, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
            expectedImageName = "TextureLinkTest_Linked";
            break;
        case TextureLinkTest_LinksRemoved:
            createAndShowDataLinkScenes<TextureLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, TextureLinkScene::DataProviderId, m_sceneIdProviderConsumer, TextureLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, TextureLinkScene::DataProviderId, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("TextureLinkTest_Linked", 0u))
            {
                return false;
            }
            testFramework.removeDataLink(m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
            testFramework.removeDataLink(m_sceneIdProviderConsumer, TextureLinkScene::DataConsumerId);
            expectedImageName = "TextureLinkTest_NoLinks";
            break;
        case TextureLinkTest_ProviderSceneUnmapped:
            createAndShowDataLinkScenes<TextureLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, TextureLinkScene::DataProviderId, m_sceneIdProviderConsumer, TextureLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, TextureLinkScene::DataProviderId, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("TextureLinkTest_Linked", 0u))
            {
                return false;
            }

            // unmapping scene will remove any links to it
            testFramework.getSceneToState(m_sceneIdProviderConsumer, ramses::RendererSceneState::Available);

            // when scene is mapped and shown again it should be in an initial state without links
            testFramework.getSceneToState(m_sceneIdProviderConsumer, ramses::RendererSceneState::Rendered);

            expectedImageName = "TextureLinkTest_NoLinks";
            break;
        case TextureLinkTest_ProvidedTextureChanges:
            createAndShowDataLinkScenes<TextureLinkScene>(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, TextureLinkScene::DataProviderId, m_sceneIdProviderConsumer, TextureLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProviderConsumer, TextureLinkScene::DataProviderId, m_sceneIdConsumer, TextureLinkScene::DataConsumerId);
            if (!testFramework.renderAndCompareScreenshot("TextureLinkTest_Linked", 0u))
            {
                return false;
            }

            {
                ramses::Scene& providerConsumerScene = testFramework.getScenesRegistry().getScene(m_sceneIdProviderConsumer);
                const auto& otherTexture = *providerConsumerScene.findObject<ramses::Texture2D>("ConsumerProviderTexture");
                ramses::Scene& providerScene = testFramework.getScenesRegistry().getScene(m_sceneIdProvider);
                providerScene.updateTextureProvider(otherTexture, TextureLinkScene::DataProviderId);
                providerScene.flush();
            }

            expectedImageName = "TextureLinkTest_ProviderTextureChanged";
            break;

        case MultiTypeLinkTest_NoLinks:
            createAndShowMultiTypeLinkScenes(testFramework);
            expectedImageName = "MultiTypeLinkTest_NoLinks";
            break;
        case MultiTypeLinkTest_Linked:
            createAndShowMultiTypeLinkScenes(testFramework);
            testFramework.createDataLink(m_sceneIdProvider, MultiTypeLinkScene::DataProviderId, m_sceneIdConsumer, MultiTypeLinkScene::DataConsumerId);
            testFramework.createDataLink(m_sceneIdProvider, MultiTypeLinkScene::TextureProviderId, m_sceneIdConsumer, MultiTypeLinkScene::TextureConsumerId);
            testFramework.createDataLink(m_sceneIdConsumer, MultiTypeLinkScene::TransformationProviderId, m_sceneIdProvider, MultiTypeLinkScene::TransformationConsumerId);
            expectedImageName = "MultiTypeLinkTest_Linked";
            break;
        default:
            assert(false && "undefined test case");
        }

        return RenderAndCompareScreenshot(testFramework, expectedImageName, 0u, expectedPixelError);
    }

    bool DataLinkingTests::RenderAndCompareScreenshot(RendererTestsFramework& testFramework, const std::string& expectedImageName, uint32_t testDisplayIdx, float expectedPixelError)
    {
        // changes to offscreen buffers are delayed by 1 frame, render one frame before taking screenshot
        testFramework.flushRendererAndDoOneLoop();
        return testFramework.renderAndCompareScreenshot(expectedImageName, testDisplayIdx, expectedPixelError);
    }

    template <typename LINKSCENE>
    void DataLinkingTests::createAndShowDataLinkScenes(RendererTestsFramework& testFramework)
    {
        m_sceneIdProvider         = testFramework.createAndShowScene<LINKSCENE>(LINKSCENE::DATA_PROVIDER             , m_cameraHigh);
        m_sceneIdProviderConsumer = testFramework.createAndShowScene<LINKSCENE>(LINKSCENE::DATA_CONSUMER_AND_PROVIDER, m_cameraMid);
        m_sceneIdConsumer         = testFramework.createAndShowScene<LINKSCENE>(LINKSCENE::DATA_CONSUMER             , m_cameraLow);
    }

    void DataLinkingTests::createAndShowMultiTypeLinkScenes(RendererTestsFramework& testFramework)
    {
        m_sceneIdConsumer = testFramework.createAndShowScene<MultiTypeLinkScene>(MultiTypeLinkScene::TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER, m_cameraLow);
        m_sceneIdProvider = testFramework.createAndShowScene<MultiTypeLinkScene>(MultiTypeLinkScene::TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER, m_cameraHigh);
    }
}
