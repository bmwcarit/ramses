//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-hmi-utils.h"
#include "ramses-client-api/RenderGroup.h"
#include "RenderGroupImpl.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/MeshNode.h"
#include "TestEffects.h"
#include "gmock/gmock.h"
#include <fstream>

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
        static Texture2D* CreateTestTexture(Scene& scene, uint8_t num)
        {
            uint8_t data[4] = { num };
            MipLevelData mipLevelData(sizeof(data), data);
            Texture2D* texture = scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache);
            EXPECT_TRUE(texture != nullptr);
            return texture;
        }

        static void SaveSceneAndResourcesToFiles(bool createMissingResources = false)
        {
            RamsesFramework frameworkForSaving;
            RamsesClient& clientForSaving(*frameworkForSaving.createClient("clientForSaving"));
            Scene* scene = clientForSaving.createScene(sceneId_t(1));

            Texture2D* texture1 = CreateTestTexture(*scene, 1u);
            Texture2D* texture2 = CreateTestTexture(*scene, 2u);
            CreateTestTexture(*scene, 3u);

            auto sampler1 = scene->createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, *texture1, 1, "sampler1");
            auto sampler2 = scene->createTextureSampler(ETextureAddressMode_Clamp, ETextureAddressMode_Clamp, ETextureSamplingMethod_Linear, ETextureSamplingMethod_Linear, *texture2, 1, "sampler2");

            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(vertexShader);
            effectDesc.setFragmentShader(fragmentShader);
            effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

            ramses::Effect* effectTex = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
            ramses::Appearance* appearance = scene->createAppearance(*effectTex);
            ramses::UniformInput textureInput;
            effectTex->findUniformInput("textureSampler", textureInput);
            appearance->setInputTexture(textureInput, *sampler1);
            ramses::UniformInput textureInput2;
            effectTex->findUniformInput("textureSampler2", textureInput2);
            appearance->setInputTexture(textureInput2, *sampler2);

            ramses::MeshNode* meshNode = scene->createMeshNode();
            meshNode->setAppearance(*appearance);

            if (createMissingResources)
                scene->destroy(*texture1);

            EXPECT_EQ(StatusOK, scene->saveToFile("scene.ramscene", false));
        }

        ARamsesHmiUtils()
            : framework()
            , client(*framework.createClient("client"))
        {
        }

        RamsesFramework framework;
        RamsesClient& client;
    };

    TEST_F(ARamsesHmiUtils, allResourcesKnownWhenLoadingScene)
    {
        SaveSceneAndResourcesToFiles();

        Scene* scene = client.loadSceneFromFile("scene.ramscene");
        ASSERT_TRUE(scene != nullptr);

        EXPECT_TRUE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, resourcesMissingWhenLoadingSceneWhichWasMissingResource)
    {
        SaveSceneAndResourcesToFiles(true);

        Scene* scene = client.loadSceneFromFile("scene.ramscene");
        ASSERT_TRUE(scene != nullptr);

        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, allResourcesKnownWhenCreatingMissingResourcesLater)
    {
        SaveSceneAndResourcesToFiles(true);

        Scene* scene = client.loadSceneFromFile("scene.ramscene");
        ASSERT_TRUE(scene != nullptr);
        EXPECT_FALSE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));

        EXPECT_TRUE(CreateTestTexture(*scene, 1u));
        EXPECT_TRUE(RamsesHMIUtils::AllResourcesForSceneKnown(*scene));
    }

    TEST_F(ARamsesHmiUtils, canDumpUnrequiredSceneObjects)
    {
        SaveSceneAndResourcesToFiles();

        Scene* scene = client.loadSceneFromFile("scene.ramscene");
        ASSERT_TRUE(scene != nullptr);

        // method only has side effects, only tests possible is that it does not crash
        RamsesHMIUtils::DumpUnrequiredSceneObjects(*scene);
    }

    TEST_F(ARamsesHmiUtils, canDumpUnrequiredSceneObjectsToFile)
    {
        SaveSceneAndResourcesToFiles();

        Scene* scene = client.loadSceneFromFile("scene.ramscene");
        ASSERT_TRUE(scene != nullptr);

        {
            std::ofstream outf("unreqObjects.txt", std::ios::out | std::ios::trunc);
            RamsesHMIUtils::DumpUnrequiredSceneObjectsToFile(*scene, outf);
        }

        std::ifstream inf("unreqObjects.txt", std::ios::in);
        std::string   content((std::istreambuf_iterator<char>(inf)), std::istreambuf_iterator<char>());
        // cannot check whole content because order of entries not deterministic
        EXPECT_THAT(content, HasSubstr("Not required ERamsesObjectType_TextureSampler name: \"sampler2\" handle: 1"));
    }

    TEST_F(ARamsesHmiUtils, canGiveTheResourceDataPoolForAnyGivenClient)
    {
        RamsesClient& client1(*framework.createClient("client1"));
        RamsesClient& client2(*framework.createClient("client2"));

        auto& rdp1 = RamsesHMIUtils::GetResourceDataPoolForClient(client1);
        auto& rdp2 = RamsesHMIUtils::GetResourceDataPoolForClient(client2);
        auto& rdp1again = RamsesHMIUtils::GetResourceDataPoolForClient(client1);

        EXPECT_EQ(&rdp1, &rdp1again);
        EXPECT_NE(&rdp1, &rdp2);
    }
}
