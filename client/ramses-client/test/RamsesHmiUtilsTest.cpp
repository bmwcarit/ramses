//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "../../ramses-client/test/SimpleSceneTopology.h"

#include "ramses-hmi-utils.h"
#include "ramses-client-api/RenderGroup.h"
#include "RenderGroupImpl.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/UniformInput.h"
#include "TestEffects.h"

using namespace testing;

namespace
{
    const char* vertexShader =
        "#version 100                                             \n"
        "uniform highp mat4 mvpMatrix;                            \n"
        "attribute vec3 a_position;                               \n"
        "attribute vec2 a_texcoord;                               \n"
        "varying vec2 v_texcoord;                                 \n"
        "                                                         \n"
        "void main()                                              \n"
        "{                                                        \n"
        "    gl_Position = mvpMatrix * vec4(a_position, 1.0);     \n"
        "    v_texcoord = a_texcoord;                             \n"
        "}                                                        \n";

    const char* fragmentShader =
        "#version 100                                                   \n"
        "uniform sampler2D textureSampler;                              \n"
        "uniform sampler2D textureSampler2;                             \n"
        "varying lowp vec2 v_texcoord;                                  \n"
        "                                                               \n"
        "void main(void)                                                \n"
        "{                                                              \n"
        "    gl_FragColor = texture2D(textureSampler, v_texcoord);      \n"
        "}                                                              \n";
}

namespace ramses
{
    class ARamsesHmiUtils : public ::testing::Test
    {
    public:
        static Texture2D* CreateTestTexture(RamsesClient& client, uint8_t num)
        {
            uint8_t data[4] = { num };
            MipLevelData mipLevelData(sizeof(data), data);
            Texture2D* texture = client.createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache);
            EXPECT_TRUE(texture != nullptr);
            return texture;
        }

        static void SaveSceneAndResourcesToFiles()
        {
            RamsesFramework frameworkForSaving;
            RamsesClient& clientForSaving(*frameworkForSaving.createClient("clientForSaving"));

            Texture2D* texture1 = CreateTestTexture(clientForSaving, 1u);
            Texture2D* texture2 = CreateTestTexture(clientForSaving, 2u);
            Texture2D* texture3 = CreateTestTexture(clientForSaving, 3u);

            Scene* scene = clientForSaving.createScene(sceneId_t(1));
            auto sampler1 = scene->createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, *texture1, 1, "sampler1");
            auto sampler2 = scene->createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, *texture2, 1, "sampler2");

            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(vertexShader);
            effectDesc.setFragmentShader(fragmentShader);
            effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

            const ramses::Effect* effectTex = clientForSaving.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
            ramses::Appearance* appearance = scene->createAppearance(*effectTex);
            ramses::UniformInput textureInput;
            effectTex->findUniformInput("textureSampler", textureInput);
            appearance->setInputTexture(textureInput, *sampler1);
            ramses::UniformInput textureInput2;
            effectTex->findUniformInput("textureSampler2", textureInput2);
            appearance->setInputTexture(textureInput2, *sampler2);

            ramses::MeshNode* meshNode = scene->createMeshNode();
            meshNode->setAppearance(*appearance);

            ResourceFileDescription resNecessary("resNecessary.ramres");
            resNecessary.add(effectTex);
            ResourceFileDescription resDesc1("res1.ramres");
            resDesc1.add(texture1);
            ResourceFileDescription resDesc2("res2.ramres");
            resDesc2.add(texture2);
            resDesc2.add(texture3);
            ResourceFileDescriptionSet resSet;
            resSet.add(resNecessary);
            resSet.add(resDesc1);
            resSet.add(resDesc2);

            EXPECT_EQ(StatusOK, clientForSaving.saveSceneToFile(*scene, "scene.ramscene", resSet, false));
        }

        ARamsesHmiUtils()
            : framework()
            , client(*framework.createClient("client"))
        {
        }

        RamsesFramework framework;
        RamsesClient& client;
    };

    TEST_F(ARamsesHmiUtils, notAllResourcesKnownWhenLoadingNoResources)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);

        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, notAllResourcesKnownWhenLoadingOnlySomeResources)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        ResourceFileDescription resDesc("res1.ramres");
        resSet.add(resDesc);

        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);

        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, notAllResourcesKnownWhenLoadingCorrectNumberButDifferentResources)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        ResourceFileDescription resDesc("res2.ramres");
        resSet.add(resDesc);

        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);

        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, allResourcesKnownWhenLoadingAllNeededResourcesWithScene)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        ResourceFileDescription resDesc1("res1.ramres");
        resSet.add(resDesc1);
        ResourceFileDescription resDesc2("res2.ramres");
        resSet.add(resDesc2);

        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);

        EXPECT_TRUE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, allResourcesKnownWhenLoadingMissingResourcesLater)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        ResourceFileDescription resDesc("res1.ramres");
        resSet.add(resDesc);

        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));

        EXPECT_EQ(StatusOK, client.loadResources(ResourceFileDescription("res2.ramres")));
        EXPECT_TRUE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, allResourcesKnownWhenCompletedWithNewlyCreatedResource)
    {
        SaveSceneAndResourcesToFiles();

        ResourceFileDescriptionSet resSet;
        ResourceFileDescription resNess("resNecessary.ramres");
        resSet.add(resNess);
        ResourceFileDescription resDesc("res1.ramres");
        resSet.add(resDesc);

        Scene* scene = client.loadSceneFromFile("scene.ramscene", resSet);
        ASSERT_TRUE(scene != nullptr);
        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));

        EXPECT_TRUE(CreateTestTexture(client, 2u) != nullptr);
        EXPECT_TRUE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

}
