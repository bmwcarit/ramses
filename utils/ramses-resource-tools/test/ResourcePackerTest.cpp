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
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/UInt16Array.h"
#include "Texture2DImpl.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "ramses-utils.h"
#include "RamsesClientImpl.h"
#include "FileUtils.h"
#include "RamsesObjectTypeUtils.h"


namespace ramses
{
    class AResourcePacker : public ::testing::Test
    {
    public:
        AResourcePacker();
        ~AResourcePacker();

    protected:
        void createResourceFiles();
        void checkEffect(const Effect& loadedEffect);
        void checkTexture(const Texture2D& loadedTexture);
        void checkIndices(const UInt16Array& loadedIndices);

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


        RamsesClient       m_client;
        Scene&             m_scene;
        Effect*            m_effect;
        Texture2D*         m_texture;
        const UInt16Array* m_indices;
    };

    AResourcePacker::AResourcePacker()
        : m_client("", m_framework)
        , m_scene(*m_client.createScene(123u, SceneConfig()))
    {
    }

    AResourcePacker::~AResourcePacker()
    {
        m_client.destroy(m_scene);
    }

    void AResourcePacker::createResourceFiles()
    {
        cleanupOutputFiles();

        ResourceFileDescriptionSet resourceFileDescriptionSet;

        // Create a Effect resource
        {
            UniformInput textColorInput;
            m_effect = RamsesUtils::CreateStandardTextEffect(m_client, textColorInput);
            ResourceFileDescription resourceFileDescription(m_outputResourceFile1.c_str());
            resourceFileDescription.add(m_effect);
            resourceFileDescriptionSet.add(resourceFileDescription);
        }

        // Create a Texture resource
        {
            uint8_t      data[4] = {0u};
            MipLevelData mipLevelData(sizeof(data), data);
            m_texture = m_client.createTexture2D(1u, 1u, ETextureFormat_RGBA8, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, "texture");
            ResourceFileDescription resourceFileDescription(m_outputResourceFile2.c_str());
            resourceFileDescription.add(m_texture);
            resourceFileDescriptionSet.add(resourceFileDescription);
        }

        // Create a UInt16Array resource
        {
            static const uint16_t indices[3] = {0, 1, 2};
            m_indices                        = m_client.createConstUInt16Array(3u, indices, ramses::ResourceCacheFlag_DoNotCache, "indices");
            ResourceFileDescription resourceFileDescription(m_outputResourceFile3.c_str());
            resourceFileDescription.add(m_indices);
            resourceFileDescriptionSet.add(resourceFileDescription);
        }

        const status_t status = m_client.saveSceneToFile(m_scene, m_outputSceneFile.c_str(), resourceFileDescriptionSet, false);
        EXPECT_EQ(StatusOK, status);
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
        EXPECT_TRUE(EEffectUniformSemantic_ModelViewProjectionMatrix == mvpMatrixInput.getSemantics());

        UniformInput textureInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findUniformInput("u_texture", textureInput));
        EXPECT_TRUE(EEffectUniformSemantic_TextTexture == textureInput.getSemantics());

        AttributeInput positionInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findAttributeInput("a_position", positionInput));
        EXPECT_TRUE(EEffectAttributeSemantic_TextPositions == positionInput.getSemantics());

        AttributeInput texcoordInput;
        EXPECT_TRUE(StatusOK == loadedEffect.findAttributeInput("a_texcoord", texcoordInput));
        EXPECT_TRUE(EEffectAttributeSemantic_TextTextureCoordinates == texcoordInput.getSemantics());

        EXPECT_EQ(m_effect->impl.getLowlevelResourceHash(), loadedEffect.impl.getLowlevelResourceHash());
    }

    void AResourcePacker::checkTexture(const Texture2D& loadedTexture)
    {
        EXPECT_EQ(m_texture->impl.getLowlevelResourceHash(), loadedTexture.impl.getLowlevelResourceHash());
    }

    void AResourcePacker::checkIndices(const UInt16Array& loadedIndices)
    {
        EXPECT_EQ(loadedIndices.impl.getElementCount(), 3u);
        EXPECT_EQ(loadedIndices.impl.getElementType(), ramses_internal::EDataType_UInt16);
        EXPECT_EQ(m_indices->impl.getLowlevelResourceHash(), loadedIndices.impl.getLowlevelResourceHash());
    }

    TEST_F(AResourcePacker, canPackMultipleResourceFilesCorrectly)
    {
        createResourceFiles();

        const char* argv[] = {"program.exe", "-ir", "res/ramses-resource-tools-resourcepackerinput.filepathesconfig", "-or", m_outputResourceFile.c_str(), NULL};
        int         argc   = sizeof(argv) / sizeof(char*) - 1;

        RamsesResourcePackerArguments arguments;
        ASSERT_TRUE(arguments.loadArguments(argc, argv));
        ASSERT_TRUE(ResourcePacker::Pack(arguments));

        RamsesClient loadedClient("ramses client", m_framework);

        ResourceFileDescription resourceFileDesc(m_outputResourceFile.c_str());
        ASSERT_TRUE(StatusOK == loadedClient.loadResources(resourceFileDesc));

        const RamsesObjectVector loadedResources = loadedClient.impl.getListOfResourceObjects();
        ASSERT_EQ(3u, loadedResources.size());

        const Effect& loadedEffect = RamsesObjectTypeUtils::ConvertTo<Effect>(*loadedResources[0]);
        checkEffect(loadedEffect);

        const Texture2D& loadedTexture = RamsesObjectTypeUtils::ConvertTo<Texture2D>(*loadedResources[1]);
        checkTexture(loadedTexture);

        const UInt16Array& loadedIndices = RamsesObjectTypeUtils::ConvertTo<UInt16Array>(*loadedResources[2]);
        checkIndices(loadedIndices);
    }
}
