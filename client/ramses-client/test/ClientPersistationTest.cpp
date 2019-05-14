//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-utils.h"

#include "ClientPersistationTest.h"
#include "RamsesObjectTestTypes.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "ResourceFileDescriptionImpl.h"
#include "Utils/File.h"
#include "Utils/BinaryFileInputStream.h"
#include "ramses-sdk-build-config.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "AnimationSystemImpl.h"

#include "ramses-client.h"
#include "ramses-utils.h"

namespace ramses
{
    using namespace testing;

    template <typename T>
    class ClientPersistationTest : public ClientPersistation {};

    TYPED_TEST_CASE(ClientPersistationTest, ResourceTypes);

    TYPED_TEST(ClientPersistationTest, canReadWriteResources)
    {
        TypeParam* resource = this->template createResource<TypeParam>("resourceName");
        ASSERT_TRUE(resource != NULL);

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const TypeParam* loadedResource = this->template getObjectForTesting<TypeParam>("resourceName");

        // Make sure the hash is not Invalid to prevent default constructed resources to pass
        EXPECT_NE(ramses_internal::ResourceContentHash::Invalid(), loadedResource->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource->impl.getLowlevelResourceHash(), loadedResource->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource->getType(), loadedResource->getType());
    }

    TYPED_TEST(ClientPersistationTest, canReadWriteResourcesWithMultipleFiles)
    {
        TypeParam* resource1 = this->template createResource<TypeParam>("resourceName1");
        ASSERT_TRUE(resource1 != NULL);

        TypeParam* resource2 = this->template createResource<TypeParam>("resourceName2");
        ASSERT_TRUE(resource2 != NULL);

        ResourceFileDescriptionSet allFileDescriptions;
        ResourceFileDescription fileDescription1("someTemporaryFile.ram");
        ResourceFileDescription fileDescription2("someOtherTemporaryFile.ram");

        fileDescription1.add(resource1);
        fileDescription2.add(resource2);

        allFileDescriptions.add(fileDescription1);
        allFileDescriptions.add(fileDescription2);


        this->doWriteReadCycle(allFileDescriptions);

        const TypeParam* loadedResource1 = this->template getObjectForTesting<TypeParam>("resourceName1");
        const TypeParam* loadedResource2 = this->template getObjectForTesting<TypeParam>("resourceName2");

        EXPECT_EQ(resource1->impl.getLowlevelResourceHash(), loadedResource1->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource2->impl.getLowlevelResourceHash(), loadedResource2->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource1->getType(), loadedResource1->getType());
        EXPECT_EQ(resource2->getType(), loadedResource2->getType());
    }

    TEST_F(ClientPersistation, canReadSameResourceMultipleTimes)
    {
        UInt16Array* resource = this->createResource<UInt16Array>("resourceName");

        ResourceFileDescription fileDescription1("someTemporaryFile.ram");
        ResourceFileDescription fileDescription2("someOtherTemporaryFile.ram");
        fileDescription1.add(resource);
        fileDescription2.add(resource);

        ResourceFileDescriptionSet resourceFiles;
        resourceFiles.add( fileDescription1 );
        resourceFiles.add( fileDescription2 );

        this->doWriteReadCycle(resourceFiles);

        const UInt16Array* loadedResource = this->getObjectForTesting<UInt16Array>("resourceName");
        EXPECT_EQ(resource->impl.getLowlevelResourceHash(), loadedResource->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource->getType(), loadedResource->getType());
    }

    TEST_F(ClientPersistation, canReadWriteIndexArray16)
    {
        UInt16Array* resource = this->createResource<UInt16Array>("resourceName");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const UInt16Array* loadedResource = this->getObjectForTesting<UInt16Array>("resourceName");
        EXPECT_EQ(resource->impl.getElementCount(), loadedResource->impl.getElementCount());
    }

    TEST_F(ClientPersistation, canReadWriteTexture2D)
    {
        Texture2D* resource = this->createResource<Texture2D>("resourceName");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const Texture2D* loadedResource = this->getObjectForTesting<Texture2D>("resourceName");
        EXPECT_EQ(resource->impl.getHeight(), loadedResource->impl.getHeight());
        EXPECT_EQ(resource->impl.getWidth(), loadedResource->impl.getWidth());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
    }

    TEST_F(ClientPersistation, canReadWriteTexture3D)
    {
        Texture3D* resource = this->createResource<Texture3D>("resourceName");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const Texture3D* loadedResource = this->getObjectForTesting<Texture3D>("resourceName");
        EXPECT_EQ(resource->impl.getHeight(), loadedResource->impl.getHeight());
        EXPECT_EQ(resource->impl.getWidth(), loadedResource->impl.getWidth());
        EXPECT_EQ(resource->impl.getDepth(), loadedResource->impl.getDepth());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
    }

    TEST_F(ClientPersistation, canReadWriteTextureCube)
    {
        TextureCube* resource = this->createResource<TextureCube>("resourceName");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const TextureCube* loadedResource = this->getObjectForTesting<TextureCube>("resourceName");
        EXPECT_EQ(resource->impl.getSize(), loadedResource->impl.getSize());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
    }

    TEST_F(ClientPersistation, canReadWriteEffect)
    {
        Effect* resource = this->createResource<Effect>("resourceName1");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const Effect* loadedResource = this->getObjectForTesting<Effect>("resourceName1");
        EXPECT_TRUE(loadedResource);
    }

    TEST_F(ClientPersistation, canLoadFromEffectFileAndSaveOutDirectly)
    {
        Effect* resource = this->createResource<Effect>("resourceName1");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        status_t status = client.saveResources(fileDescription, false);
        EXPECT_EQ(StatusOK, status);

        // load created effect resource in another client
        ramses::RamsesFramework anotherFramework;
        ramses::RamsesClient anotherClient("ramses client", anotherFramework);
        EXPECT_EQ(StatusOK, anotherClient.loadResources(fileDescription));

        // write out the loaded resource immediately
        ramses::ResourceFileDescription outputFile("someOtherTempFile.ram");
        const ramses::RamsesObjectVector resourceObjects = anotherClient.impl.getListOfResourceObjects();
        for(const auto& resObj : resourceObjects)
        {
            const ramses::Resource* res = RamsesUtils::TryConvert<ramses::Resource>(*resObj);
            ASSERT_TRUE(NULL != res);
            outputFile.impl->m_resources.push_back(res);
        }

        EXPECT_EQ(StatusOK, anotherClient.saveResources(outputFile, false));
    }

    TEST_F(ClientPersistation, canReadWriteIndexArray32)
    {
        UInt32Array* resource = this->createResource<UInt32Array>("resourceName");

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        this->doWriteReadCycle(fileDescription);

        const UInt32Array* loadedResource = this->getObjectForTesting<UInt32Array>("resourceName");
        EXPECT_EQ(resource->impl.getElementCount(), loadedResource->impl.getElementCount());
    }

    TEST_F(ClientPersistation, saveResourcesToFile_InvalidFilename)
    {
        ResourceFileDescription fileDescription("?XYZ:/dummyFile");
        ramses::status_t status = client.saveResources(fileDescription, false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ClientPersistation, saveResourcesToFile_NoFilename)
    {
        ResourceFileDescription fileDescription(NULL);
        ramses::status_t status = client.saveResources(fileDescription, false);
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ClientPersistation, saveResourcesToFile_FileExists)
    {
        ResourceFileDescription fileDescription("dummyFile.dat");

        ramses::status_t status = client.saveResources(fileDescription, false);
        EXPECT_EQ(ramses::StatusOK, status);

        ramses_internal::File file(fileDescription.getFilename());
        EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, file.remove());
    }

    TEST_F(ClientPersistation, loadResourcesFromFile_InvalidFilename)
    {
        ramses::status_t status = m_clientForLoading.loadResources(ResourceFileDescription("?XYZ:/dummyFile"));
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ClientPersistation, loadResourcesFromFile_NoFilename)
    {
        ramses::status_t status = m_clientForLoading.loadResources(ResourceFileDescription(NULL));
        EXPECT_NE(ramses::StatusOK, status);
    }

    TEST_F(ClientPersistation, loadResourcesFromFile_NonexistantFilename)
    {
        ramses::status_t status = m_clientForLoading.loadResources(ResourceFileDescription("ZEGETWTWAGTGSDGEg_thisfilename_in_this_directory_should_not_exist_DSAFDSFSTEZHDXHB"));
        EXPECT_NE(ramses::StatusOK, status);
    }

    static void checkVersionStrings(ramses_internal::BinaryFileInputStream& stream)
    {
        ramses_internal::String expectedVersionString =
            ramses_internal::String("[RamsesVersion:") +
            ramses_internal::String(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING) +
            ramses_internal::String("]\n[GitHash:") +
            ramses_internal::String(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH) +
            ramses_internal::String("]\n");

        const ramses_internal::UInt32 stringSize = static_cast<ramses_internal::UInt32>(expectedVersionString.getLength());

        ramses_internal::Char* versionStringInFile = new ramses_internal::Char[stringSize + 1];
        stream.read(versionStringInFile, stringSize);
        versionStringInFile[stringSize] = '\0';

        EXPECT_STREQ(versionStringInFile, expectedVersionString.c_str());

        delete[] versionStringInFile;
    }

    TEST_F(ClientPersistation, storesRamsesVersionToSceneFile)
    {
        // Save empty scene and empty resources
        {
            ramses::Scene* emptyScene = client.createScene(15);
            ramses::ResourceFileDescriptionSet emptyResourceFileInformation;
            ramses::ResourceFileDescription emptyAsset("emptyResourceAsset.ramres");
            emptyResourceFileInformation.add(emptyAsset);

            ramses::status_t status = client.saveSceneToFile(*emptyScene, "emptyScene.ramscene", emptyResourceFileInformation, false);
            ASSERT_EQ(ramses::StatusOK, status);
            client.destroy(*emptyScene);
        }

        // Load and check version strings
        {
            ramses_internal::File sceneFile("emptyScene.ramscene");
            ramses_internal::BinaryFileInputStream sceneStream(sceneFile);

            checkVersionStrings(sceneStream);

            ramses_internal::File resourceFile("emptyResourceAsset.ramres");
            ramses_internal::BinaryFileInputStream resourceStream(resourceFile);

            checkVersionStrings(resourceStream);
        }
    }

    template<typename ObjectType>
    const ObjectType* getObjectFromScene(Scene* scene, const char* name)
    {
        const RamsesObject* objectPerName = scene->findObjectByName(name);
        EXPECT_STREQ(name, objectPerName->getName());

        const ObjectType* specificObject = RamsesUtils::TryConvert<ObjectType>(*objectPerName);
        EXPECT_TRUE(0 != specificObject);
        return specificObject;
    }

    TEST_F(ClientPersistation, shareResourcesBetweenScenes)
    {
        Effect* effect1 = TestEffects::CreateTestEffect(client, "effect1"); // used by small and big scene
        Effect* effect2 = TestEffects::CreateTestEffect(client, "effect2"); // only used by big scene, important: use different name to create different resource ids

        // create small scene with a single appearance/effect
        Scene* smallScene = client.createScene(1);
        Appearance* appearance = smallScene->createAppearance(*effect1, "appearance1");

        ResourceFileDescription resourcesSmall("smallSceneResources.ramres");
        resourcesSmall.add(effect1);
        ResourceFileDescriptionSet resourceSetSmall;
        resourceSetSmall.add(resourcesSmall);
        EXPECT_EQ(StatusOK, client.saveSceneToFile(*smallScene, "smallScene.ramscene", resourceSetSmall, false));

        // create big scene with two appearances, also using the effect from small scene
        Scene* bigScene = client.createScene(2);
        Appearance* appearance1 = bigScene->createAppearance(*effect1, "appearance1");
        Appearance* appearance2 = bigScene->createAppearance(*effect2, "appearance2");

        ResourceFileDescription resourcesBig("bigSceneResources.ramres");
        resourcesBig.add(effect2);
        ResourceFileDescriptionSet resourceSetBig;
        resourceSetBig.add(resourcesBig);
        EXPECT_EQ(StatusOK, client.saveSceneToFile(*bigScene, "bigScene.ramscene", resourceSetBig, false));

        // load small scene
        Scene *loadedSmallScene = m_clientForLoading.loadSceneFromFile("smallScene.ramscene", resourceSetSmall);
        ASSERT_TRUE(loadedSmallScene != 0);
        EXPECT_TRUE(m_clientForLoading.findResourceById(effect1->getResourceId()) != 0);
        EXPECT_FALSE(m_clientForLoading.findResourceById(effect2->getResourceId()) != 0);

        const Appearance* loadedSmallAppearance1 = getObjectFromScene<Appearance>(loadedSmallScene, "appearance1");
        EXPECT_EQ(loadedSmallAppearance1->getEffect().getResourceId(), appearance->getEffect().getResourceId());

        // load big scene, all appearances can find their effects
        Scene *loadedBigScene = m_clientForLoading.loadSceneFromFile("bigScene.ramscene", resourceSetBig);
        ASSERT_TRUE(loadedBigScene != 0);
        EXPECT_TRUE(m_clientForLoading.findResourceById(effect2->getResourceId()) != 0);

        const Appearance* loadedBigAppearance1 = getObjectFromScene<Appearance>(loadedBigScene, "appearance1");
        const Appearance* loadedBigAppearance2 = getObjectFromScene<Appearance>(loadedBigScene, "appearance2");
        EXPECT_EQ(loadedBigAppearance1->getEffect().getResourceId(), appearance1->getEffect().getResourceId());
        EXPECT_EQ(loadedBigAppearance2->getEffect().getResourceId(), appearance2->getEffect().getResourceId());
    }

    TEST_F(ClientPersistation, compressedFileIsSmallerThanUncompressed)
    {
        const std::vector<uint16_t> data(1000u, 0u);
        const UInt16Array* resource = this->client.createConstUInt16Array(static_cast<uint32_t>(data.size()), data.data());

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);

        EXPECT_EQ(StatusOK, client.saveResources(fileDescription, false));

        ramses_internal::File file(fileDescription.getFilename());
        EXPECT_TRUE(file.exists());
        ramses_internal::UInt fileSize = 0;
        EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, file.getSizeInBytes(fileSize));

        EXPECT_EQ(StatusOK, client.saveResources(fileDescription, true));

        ramses_internal::File file2(fileDescription.getFilename());
        EXPECT_TRUE(file2.exists());
        ramses_internal::UInt fileSize2 = 0;
        EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, file2.getSizeInBytes(fileSize2));

        EXPECT_GT(fileSize, fileSize2);
    }

    TEST_F(ClientPersistation, compressedFileIsSmallerThanUncompressedWhenUsingSaveSceneToFile)
    {
        Scene* scene = client.createScene(1);
        const std::vector<uint16_t> data(1000u, 0u);
        const UInt16Array* resource = this->client.createConstUInt16Array(static_cast<uint32_t>(data.size()), data.data());

        ResourceFileDescription fileDescription("someTempararyFile.ram");
        fileDescription.add(resource);
        ResourceFileDescriptionSet fileDescriptionSet;
        fileDescriptionSet.add(fileDescription);
        EXPECT_EQ(StatusOK, client.saveSceneToFile(*scene, "testscene.ramscene", fileDescriptionSet, false));

        ramses_internal::File file(fileDescription.getFilename());
        EXPECT_TRUE(file.exists());
        ramses_internal::UInt uncompressedFileSize = 0;
        EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, file.getSizeInBytes(uncompressedFileSize));

        EXPECT_EQ(StatusOK, client.saveSceneToFile(*scene, "testscene2.ramscene", fileDescriptionSet, true));

        ramses_internal::File file2(fileDescription.getFilename());
        EXPECT_TRUE(file2.exists());
        ramses_internal::UInt compressedFileSize = 0;
        EXPECT_EQ(ramses_internal::EStatus_RAMSES_OK, file2.getSizeInBytes(compressedFileSize));

        EXPECT_GT(uncompressedFileSize, compressedFileSize);
    }

    bool compareBinaryFiles(char const* fileName1, char const* fileName2)
    {
        using namespace ramses_internal;
        File file1(fileName1);
        file1.open(EFileMode_ReadOnlyBinary);
        File file2(fileName2);
        file2.open(EFileMode_ReadOnlyBinary);

        if (!file1.isOpen() || !file2.isOpen())
        {
            return false;
        }

        UInt length1 = 0u;
        UInt length2 = 0u;
        file1.getSizeInBytes(length1);
        file2.getSizeInBytes(length2);
        if (length1 != length2)
        {
            return false;
        }

        const UInt bufferLength = 1024*1024;
        std::vector<Char> buf1(bufferLength);
        std::vector<Char> buf2(bufferLength);

        UInt offset = 0;
        UInt dummy = 0;
        bool equal = true;
        while (offset < length1 && equal)
        {
            UInt left =  min(length1 - offset, bufferLength);
            file1.read(buf1.data(), left, dummy);
            file2.read(buf2.data(), left, dummy);
            equal = (PlatformMemory::Compare(buf1.data(), buf2.data(), left) == 0);
            offset += left;
        }

        return equal;
    }

    TEST_F(ClientPersistation, savedFilesAreConsistent)
    {
        for (ramses_internal::String name : { "ts1.ram", "ts2.ram", "ts3.ram", "ts4.ram", "ts5.ram", "ts6.ram" })
        {
            Scene* scene = client.createScene(1u);
            Effect* resourceEffect1 = this->createResource<Effect>("resourceName1");
            Effect* resourceEffect2 = this->createResource<Effect>("resourceName2");
            TextureCube* resourceTex = this->createResource<TextureCube>("resourceTex");

            ResourceFileDescription fileDescription((name + "res").c_str());
            fileDescription.add(resourceEffect1);
            fileDescription.add(resourceEffect2);
            fileDescription.add(resourceTex);
            ResourceFileDescriptionSet fileDescriptionSet;
            fileDescriptionSet.add(fileDescription);
            EXPECT_EQ(StatusOK, client.saveSceneToFile(*scene, (name + "scene").c_str(), fileDescriptionSet, false));
            client.destroy(*scene);
        }

        for (ramses_internal::String name : { "ts2.ram", "ts3.ram", "ts4.ram", "ts5.ram", "ts6.ram" })
        {
            EXPECT_TRUE(compareBinaryFiles("ts1.ramres", (name + "res").c_str()));
            EXPECT_TRUE(compareBinaryFiles("ts1.ramscene", (name + "scene").c_str()));
        }
    }
}
