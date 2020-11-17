//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesResourcePackerArguments.h"
#include "ResourcePacker.h"
#include "gtest/gtest.h"

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/ArrayResource.h"
#include "Texture2DImpl.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "ramses-utils.h"
#include "RamsesClientImpl.h"
#include "FileUtils.h"
#include "RamsesObjectTypeUtils.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-hmi-utils.h"


namespace ramses
{
    class AResourcePacker : public ::testing::Test
    {
    public:
        AResourcePacker();
        ~AResourcePacker();

    protected:
        void createResourceFiles(std::vector<resourceId_t>& resources);
        void checkEffect(const Effect& loadedEffect);
        void checkTexture(const Texture2D& loadedTexture);
        void checkIndices(const ArrayResource& loadedIndices);

    private:
        void        cleanupOutputFiles();
        static void RemoveFile(const ramses_internal::String& fileName);

    protected:
        const ramses_internal::String m_outputResourceFile = "res/ramses-resource-tools-resourcepacker.res";
        RamsesFramework               m_framework;

    private:
        const ramses_internal::String m_outputResourceFile1 = "res/ramses-resource-tools-resourcepacker1.res";
        const ramses_internal::String m_outputResourceFile2 = "res/ramses-resource-tools-resourcepacker2.res";
        const ramses_internal::String m_outputResourceFile3 = "res/ramses-resource-tools-resourcepacker3.res";
        const ramses_internal::String m_outputSceneFile     = "res/ramses-resource-tools-resourcepacker.ramses";

        RamsesClient& m_client;
        Scene& m_scene;
        Effect* m_effect;
        Texture2D* m_texture;
        ArrayResource* m_indices;
    };

    AResourcePacker::AResourcePacker()
        : m_client(*m_framework.createClient(""))
        , m_scene(*m_client.createScene(ramses::sceneId_t(123u), SceneConfig()))
    {
    }

    AResourcePacker::~AResourcePacker()
    {
        m_client.destroy(m_scene);
    }

    void AResourcePacker::createResourceFiles(std::vector<resourceId_t>& resources)
    {
        cleanupOutputFiles();

        // Create a Effect resource
        {
            ramses::EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "precision highp float;\n"
                "uniform highp mat4 mvpMatrix;\n"
                "attribute vec2 a_position; \n"
                "attribute vec2 a_texcoord; \n"
                "\n"
                "varying vec2 v_texcoord; \n"
                "\n"
                "void main()\n"
                "{\n"
                "  v_texcoord = a_texcoord; \n"
                "  gl_Position = mvpMatrix * vec4(a_position, 0.0, 1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "precision highp float;\n"
                "uniform sampler2D u_texture; \n"
                "uniform vec4 u_color; \n"
                "varying vec2 v_texcoord; \n"
                "\n"
                "void main(void)\n"
                "{\n"
                "  float a = texture2D(u_texture, v_texcoord).r; \n"
                "  gl_FragColor = vec4(u_color.x, u_color.y, u_color.z, a); \n"
                "}\n");

            effectDesc.setAttributeSemantic("a_position", EEffectAttributeSemantic::TextPositions);
            effectDesc.setAttributeSemantic("a_texcoord", EEffectAttributeSemantic::TextTextureCoordinates);
            effectDesc.setUniformSemantic("u_texture", EEffectUniformSemantic::TextTexture);
            effectDesc.setUniformSemantic("mvpMatrix", EEffectUniformSemantic::ModelViewProjectionMatrix);

            m_effect = m_scene.createEffect(effectDesc);

            Scene& tempScene(*m_client.createScene(sceneId_t{ 0xf00 }));
            auto effect = tempScene.createEffect(effectDesc);
            resources.push_back(effect->getResourceId());
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(tempScene, m_outputResourceFile1.c_str(), false));
            m_client.destroy(tempScene);
        }

        // Create a Texture resource
        {
            uint8_t      data[4] = {0u};
            MipLevelData mipLevelData(sizeof(data), data);
            m_texture = m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "texture");

            Scene& tempScene(*m_client.createScene(sceneId_t{ 0xf00 }));
            auto texture = tempScene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ResourceCacheFlag_DoNotCache, "texture");
            resources.push_back(texture->getResourceId());
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(tempScene, m_outputResourceFile2.c_str(), false));
            m_client.destroy(tempScene);
        }

        // Create an array resource
        {
            static const uint16_t indicesRaw[3] = {0, 1, 2};
            m_indices                        = m_scene.createArrayResource(ramses::EDataType::UInt16, 3u, indicesRaw, ramses::ResourceCacheFlag_DoNotCache, "indices");

            Scene& tempScene(*m_client.createScene(sceneId_t{ 0xf00 }));
            auto indices = tempScene.createArrayResource(ramses::EDataType::UInt16, 3u, indicesRaw, ramses::ResourceCacheFlag_DoNotCache, "indices");
            resources.push_back(indices->getResourceId());
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(tempScene, m_outputResourceFile3.c_str(), false));
            m_client.destroy(tempScene);
        }
    }

    void AResourcePacker::RemoveFile(const ramses_internal::String& fileName)
    {
        FileUtils::RemoveFileIfExist(fileName.c_str());
        EXPECT_FALSE(FileUtils::FileExists(fileName.c_str()));
    }

    void AResourcePacker::cleanupOutputFiles()
    {
        RemoveFile(m_outputResourceFile1);
        RemoveFile(m_outputResourceFile2);
        RemoveFile(m_outputResourceFile3);
        RemoveFile(m_outputResourceFile);
        RemoveFile(m_outputSceneFile);
    }

    void AResourcePacker::checkEffect(const Effect& loadedEffect)
    {
        UniformInput mvpMatrixInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findUniformInput("mvpMatrix", mvpMatrixInput));
        EXPECT_TRUE(EEffectUniformSemantic::ModelViewProjectionMatrix == mvpMatrixInput.getSemantics());

        UniformInput textureInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findUniformInput("u_texture", textureInput));
        EXPECT_TRUE(EEffectUniformSemantic::TextTexture == textureInput.getSemantics());

        AttributeInput positionInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findAttributeInput("a_position", positionInput));
        EXPECT_TRUE(EEffectAttributeSemantic::TextPositions == positionInput.getSemantics());

        AttributeInput texcoordInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findAttributeInput("a_texcoord", texcoordInput));
        EXPECT_TRUE(EEffectAttributeSemantic::TextTextureCoordinates == texcoordInput.getSemantics());

        EXPECT_EQ(m_effect->impl.getLowlevelResourceHash(), loadedEffect.impl.getLowlevelResourceHash());
    }

    void AResourcePacker::checkTexture(const Texture2D& loadedTexture)
    {
        EXPECT_EQ(m_texture->impl.getLowlevelResourceHash(), loadedTexture.impl.getLowlevelResourceHash());
    }

    void AResourcePacker::checkIndices(const ArrayResource& loadedIndices)
    {
        EXPECT_EQ(loadedIndices.impl.getElementCount(), 3u);
        EXPECT_EQ(loadedIndices.impl.getElementType(), ramses::EDataType::UInt16);
        EXPECT_EQ(m_indices->impl.getLowlevelResourceHash(), loadedIndices.impl.getLowlevelResourceHash());
    }

    TEST_F(AResourcePacker, canPackMultipleResourceFilesCorrectly)
    {
        std::vector<resourceId_t> vec;
        createResourceFiles(vec);

        const char* argv[] = {"program.exe", "-ir", "res/ramses-resource-tools-resourcepackerinput.filepathesconfig", "-or", m_outputResourceFile.c_str(), nullptr};
        int         argc   = sizeof(argv) / sizeof(char*) - 1;

        RamsesResourcePackerArguments arguments;
        ASSERT_TRUE(arguments.loadArguments(argc, argv));
        ASSERT_TRUE(ResourcePacker::Pack(arguments));

        RamsesClient& loadedClient(*m_framework.createClient("ramses client"));
        Scene& scene(*loadedClient.createScene(sceneId_t{ 0xf00 }));

        ASSERT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(loadedClient).addResourceDataFile(m_outputResourceFile.c_str()));

        ASSERT_EQ(vec.size(), 3u);
        ASSERT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(loadedClient).createResourceForScene(scene, vec[0]));
        ASSERT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(loadedClient).createResourceForScene(scene, vec[1]));
        ASSERT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(loadedClient).createResourceForScene(scene, vec[2]));

        const Effect& loadedEffect = RamsesObjectTypeUtils::ConvertTo<Effect>(*scene.getResource(vec[0]));
        checkEffect(loadedEffect);

        const Texture2D& loadedTexture = RamsesObjectTypeUtils::ConvertTo<Texture2D>(*scene.getResource(vec[1]));
        checkTexture(loadedTexture);

        const ArrayResource& loadedIndices = RamsesObjectTypeUtils::ConvertTo<ArrayResource>(*scene.getResource(vec[2]));
        checkIndices(loadedIndices);
    }
}
