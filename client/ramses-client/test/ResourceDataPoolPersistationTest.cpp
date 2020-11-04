//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client.h"
#include "ramses-utils.h"

#include "ResourceDataPoolPersistationTest.h"
#include "RamsesObjectTestTypes.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "TextureCubeImpl.h"
#include "ramses-sdk-build-config.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses
{
    using namespace testing;

    template <typename T>
    class ResourceDataPoolPersistationTest : public ResourceDataPoolPersistation {};

    TYPED_TEST_SUITE(ResourceDataPoolPersistationTest, ResourceTypes);

    TYPED_TEST(ResourceDataPoolPersistationTest, canReadWriteResources)
    {
        TypeParam* resource = this->template createResource<TypeParam>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TypeParam* loadedResource = this->template getObjectForTesting<TypeParam>(resource->getResourceId(), "resourceName");

        // Make sure the hash is not Invalid to prevent default constructed resources to pass
        EXPECT_NE(ramses_internal::ResourceContentHash::Invalid(), loadedResource->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource->impl.getLowlevelResourceHash(), loadedResource->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource->getType(), loadedResource->getType());
    }

    TYPED_TEST(ResourceDataPoolPersistationTest, canReadWriteMultipleResources)
    {
        TypeParam* resource1 = this->template createResource<TypeParam>("resourceName1");
        ASSERT_TRUE(resource1 != nullptr);

        TypeParam* resource2 = this->template createResource<TypeParam>("resourceName2");
        ASSERT_TRUE(resource2 != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TypeParam* loadedResource1 = this->template getObjectForTesting<TypeParam>(resource1->getResourceId(), "resourceName1");
        const TypeParam* loadedResource2 = this->template getObjectForTesting<TypeParam>(resource2->getResourceId(), "resourceName2");

        EXPECT_EQ(resource1->impl.getLowlevelResourceHash(), loadedResource1->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource2->impl.getLowlevelResourceHash(), loadedResource2->impl.getLowlevelResourceHash());
        EXPECT_EQ(resource1->getType(), loadedResource1->getType());
        EXPECT_EQ(resource2->getType(), loadedResource2->getType());
    }

    TYPED_TEST(ResourceDataPoolPersistationTest, failsToCreateResourceMultipleTimes)
    {
        TypeParam* resource = this->template createResource<TypeParam>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TypeParam* loadedResource = this->template getObjectForTesting<TypeParam>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getLowlevelResourceHash(), loadedResource->impl.getLowlevelResourceHash());

        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(this->m_clientForLoading).createResourceForScene(*this->m_sceneLoaded, resource->getResourceId()));
    }

    TYPED_TEST(ResourceDataPoolPersistationTest, canGetResourceMultipleTimes)
    {
        TypeParam* resource = this->template createResource<TypeParam>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TypeParam* loadedResource = this->template getObjectForTesting<TypeParam>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getLowlevelResourceHash(), loadedResource->impl.getLowlevelResourceHash());

        auto res2 = this->m_sceneLoaded->getResource(resource->getResourceId());
        EXPECT_EQ(loadedResource, res2);
    }

    TYPED_TEST(ResourceDataPoolPersistationTest, cantGetResourceBeforeAddingFileToResourcePool)
    {
        TypeParam* resource = this->template createResource<TypeParam>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        if (!this->m_sceneLoaded)
            this->m_sceneLoaded = this->m_clientForLoading.createScene(sceneId_t{ 0xf00b42 });
        EXPECT_FALSE(this->m_sceneLoaded->getResource(resource->getResourceId()));
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteArrayResource)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const ArrayResource* loadedResource = this->template getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->getNumberOfElements(), loadedResource->getNumberOfElements());
        EXPECT_EQ(resource->getDataType(), loadedResource->getDataType());
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteArrayResourceWithOtherTypeAndSize)
    {
        const float data[6] = { .0f };
        ArrayResource* resource = m_scene.createArrayResource(EDataType::Vector3F, 2, data, ResourceCacheFlag_DoNotCache, "resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const ArrayResource* loadedResource = this->getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->getNumberOfElements(), loadedResource->getNumberOfElements());
        EXPECT_EQ(resource->getDataType(), loadedResource->getDataType());
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteTexture2D)
    {
        Texture2D* resource = this->createResource<Texture2D>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const Texture2D* loadedResource = this->getObjectForTesting<Texture2D>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getHeight(), loadedResource->impl.getHeight());
        EXPECT_EQ(resource->impl.getWidth(), loadedResource->impl.getWidth());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelRed, loadedResource->impl.getTextureSwizzle().channelRed);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelGreen, loadedResource->impl.getTextureSwizzle().channelGreen);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelBlue, loadedResource->impl.getTextureSwizzle().channelBlue);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelAlpha, loadedResource->impl.getTextureSwizzle().channelAlpha);
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteTexture2DwithNonDefaultSwizzle)
    {
        const TextureSwizzle swizzle = {ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue};
        const uint8_t data[4] = { 0u };
        const MipLevelData mipLevelData(sizeof(data), data);
        Texture2D* resource = m_scene.createTexture2D(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, "resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const Texture2D* loadedResource = this->getObjectForTesting<Texture2D>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getHeight(), loadedResource->impl.getHeight());
        EXPECT_EQ(resource->impl.getWidth(), loadedResource->impl.getWidth());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelRed, loadedResource->impl.getTextureSwizzle().channelRed);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelGreen, loadedResource->impl.getTextureSwizzle().channelGreen);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelBlue, loadedResource->impl.getTextureSwizzle().channelBlue);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelAlpha, loadedResource->impl.getTextureSwizzle().channelAlpha);
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteTexture3D)
    {
        Texture3D* resource = this->createResource<Texture3D>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const Texture3D* loadedResource = this->getObjectForTesting<Texture3D>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getHeight(), loadedResource->impl.getHeight());
        EXPECT_EQ(resource->impl.getWidth(), loadedResource->impl.getWidth());
        EXPECT_EQ(resource->impl.getDepth(), loadedResource->impl.getDepth());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteTextureCube)
    {
        TextureCube* resource = this->createResource<TextureCube>("resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TextureCube* loadedResource = this->getObjectForTesting<TextureCube>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getSize(), loadedResource->impl.getSize());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelRed, loadedResource->impl.getTextureSwizzle().channelRed);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelGreen, loadedResource->impl.getTextureSwizzle().channelGreen);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelBlue, loadedResource->impl.getTextureSwizzle().channelBlue);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelAlpha, loadedResource->impl.getTextureSwizzle().channelAlpha);
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteTextureCubeWithNonDefaultSwizzle)
    {
        const TextureSwizzle swizzle = { ETextureChannelColor::Alpha, ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue };
        const uint8_t data[4] = { 0u };
        const CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
        TextureCube* resource = m_scene.createTextureCube(ETextureFormat::RGBA8, 1u, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, "resourceName");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const TextureCube* loadedResource = this->getObjectForTesting<TextureCube>(resource->getResourceId(), "resourceName");
        EXPECT_EQ(resource->impl.getSize(), loadedResource->impl.getSize());
        EXPECT_EQ(resource->impl.getTextureFormat(), loadedResource->impl.getTextureFormat());
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelRed, loadedResource->impl.getTextureSwizzle().channelRed);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelGreen, loadedResource->impl.getTextureSwizzle().channelGreen);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelBlue, loadedResource->impl.getTextureSwizzle().channelBlue);
        EXPECT_EQ(resource->impl.getTextureSwizzle().channelAlpha, loadedResource->impl.getTextureSwizzle().channelAlpha);
    }

    TEST_F(ResourceDataPoolPersistation, canReadWriteEffect)
    {
        Effect* resource = this->createResource<Effect>("resourceName1");

        this->doWriteReadCycle("someTemporaryFile.ram");

        const Effect* loadedResource = this->getObjectForTesting<Effect>(resource->getResourceId(), "resourceName1");
        EXPECT_TRUE(loadedResource);
    }

    TEST_F(ResourceDataPoolPersistation, canLoadFromEffectFileAndSaveOutDirectly)
    {
        Effect* resource = this->createResource<Effect>("resourceName1");

        EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "someTemporaryFile.ram", false));

        // load created effect resource in another client
        ramses::RamsesFramework anotherFramework;
        ramses::RamsesClient& anotherClient(*anotherFramework.createClient("ramses client"));
        auto& scene(*anotherClient.createScene(sceneId_t{ 0xf00 }));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(anotherClient).addResourceDataFile("someTemporaryFile.ram"));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(anotherClient).createResourceForScene(scene, resource->getResourceId()));

        // write out the loaded resource immediately
        EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(scene, "someOtherTempFile.ram", false));
    }

    TEST_F(ResourceDataPoolPersistation, saveResourcesToFile_InvalidFilename)
    {
        EXPECT_FALSE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "?XYZ:/dummyFile", false));
    }

    TEST_F(ResourceDataPoolPersistation, saveResourcesToFile_NoFilename)
    {
        EXPECT_FALSE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "", false));
    }

    TEST_F(ResourceDataPoolPersistation, saveResourcesToFile_FileExists)
    {
        EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "dummyFile.dat", false));

        ramses_internal::File file("dummyFile.dat");
        EXPECT_TRUE(file.remove());
    }

    TEST_F(ResourceDataPoolPersistation, addResourceDataFile_InvalidFilename)
    {
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).addResourceDataFile("?XYZ:/dummyFile"));
    }

    TEST_F(ResourceDataPoolPersistation, addResourceDataFile_NoFilename)
    {
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).addResourceDataFile(""));
    }

    TEST_F(ResourceDataPoolPersistation, addResourceDataFile_NonexistantFilename)
    {
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).addResourceDataFile("ZEGETWTWAGTGSDGEg_thisfilename_in_this_directory_should_not_exist_DSAFDSFSTEZHDXHB"));
    }

    static void checkVersionStrings(ramses_internal::BinaryFileInputStream& stream)
    {
        ramses_internal::String expectedVersionString =
            ramses_internal::String("[RamsesVersion:") +
            ramses_internal::String(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING) +
            ramses_internal::String("]\n[GitHash:") +
            ramses_internal::String(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH) +
            ramses_internal::String("]\n");

        const ramses_internal::UInt32 stringSize = static_cast<ramses_internal::UInt32>(expectedVersionString.size());

        ramses_internal::Char* versionStringInFile = new ramses_internal::Char[stringSize + 1];
        stream.read(versionStringInFile, stringSize);
        versionStringInFile[stringSize] = '\0';

        EXPECT_STREQ(versionStringInFile, expectedVersionString.c_str());

        delete[] versionStringInFile;
    }

    TEST_F(ResourceDataPoolPersistation, storesRamsesVersionToSceneAndResourceFile)
    {
        // Save empty scene and empty resources
        {
            ramses::Scene* emptyScene = client.createScene(sceneId_t(15u));
            ramses::status_t status = emptyScene->saveToFile("emptyScene.ramscene", false);
            ASSERT_EQ(ramses::StatusOK, status);

            RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(*emptyScene, "emptyResourceAsset.ramres", false);
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
        EXPECT_TRUE(nullptr != specificObject);
        return specificObject;
    }

    TEST_F(ResourceDataPoolPersistation, compressedFileIsSmallerThanUncompressed)
    {
        const std::vector<uint16_t> data(1000u, 0u);
        this->m_scene.createArrayResource(EDataType::UInt16, static_cast<uint32_t>(data.size()), data.data());

        EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "someTemporaryFile.ram", false));

        ramses_internal::File file("someTemporaryFile.ram");
        EXPECT_TRUE(file.exists());
        ramses_internal::UInt fileSize = 0;
        EXPECT_TRUE(file.getSizeInBytes(fileSize));
        EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, "someTemporaryFile.ram", true));

        ramses_internal::File file2("someTemporaryFile.ram");
        EXPECT_TRUE(file2.exists());
        ramses_internal::UInt fileSize2 = 0;
        EXPECT_TRUE(file2.getSizeInBytes(fileSize2));

        EXPECT_GT(fileSize, fileSize2);
    }

    TEST_F(ResourceDataPoolPersistation, savedResourceDataFilesAreConsistent)
    {
        this->createResource<Effect>("resourceName1");
        this->createResource<Effect>("resourceName2");
        this->createResource<TextureCube>("resourceTex");
        this->createResource<TextureCube>("resourceTex");

        for (ramses_internal::String name : { "ts1.ramres", "ts2.ramres", "ts3.ramres", "ts4.ramres", "ts5.ramres", "ts6.ramres" })
        {
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(m_scene, name.c_str(), false));
        }

        for (ramses_internal::String name : { "ts2.ramres", "ts3.ramres", "ts4.ramres", "ts5.ramres", "ts6.ramres" })
        {
            EXPECT_TRUE(ramses::ClientTestUtils::CompareBinaryFiles("ts1.ramres", name.c_str()));
        }
    }

    class ResourceSavingTestClient : public LocalTestClientWithSceneAndAnimationSystem
    {
    public:
        template<typename ResourceType>
        ResourceType* createResource(const char* name)
        {
            return m_creationHelper.createObjectOfType<ResourceType>(name);
        }
    };

    TEST(AResourceDataPoolPersistation, keepsResourceDataFilesConsistentAcrossMultipleClients)
    {
        for (ramses_internal::String name : { "ts1.ramres", "ts2.ramres", "ts3.ramres", "ts4.ramres", "ts5.ramres", "ts6.ramres" })
        {
            ResourceSavingTestClient testClient;
            testClient.createResource<ArrayResource>("resourceTex");
            testClient.createResource<ArrayResource>("resourceTex");
            testClient.createResource<Effect>("resourceName1");
            testClient.createResource<Effect>("resourceName2");
            testClient.createResource<Texture2D>("resourceTex");
            testClient.createResource<Texture3D>("resourceTex");
            testClient.createResource<TextureCube>("resourceTex");
            EXPECT_TRUE(RamsesHMIUtils::SaveResourcesOfSceneToResourceFile(testClient.getScene(), name.c_str(), false));
        }

        for (ramses_internal::String name : { "ts2.ramres", "ts3.ramres", "ts4.ramres", "ts5.ramres", "ts6.ramres" })
        {
            EXPECT_TRUE(ramses::ClientTestUtils::CompareBinaryFiles("ts1.ramres", name.c_str()));
        }
    }

    TEST_F(ResourceDataPoolPersistation, doesNotLoadResourceOnResourceFileClose)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");
        this->template getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");

        auto llhash = static_cast<Resource*>(resource)->impl.getLowlevelResourceHash();
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash));

        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile.ram"));
        Scene& scene2(*m_clientForLoading.createScene(sceneId_t{ 0x00110101 }));
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(scene2, resource->getResourceId()));
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash));
    }

    TEST_F(ResourceDataPoolPersistation, canForceLoadLowLevelResource)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");
        this->template getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");

        auto llhash = static_cast<Resource*>(resource)->impl.getLowlevelResourceHash();
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash));

        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).forceLoadResourcesFromResourceDataFile("someTemporaryFile.ram"));
        Scene& scene2(*m_clientForLoading.createScene(sceneId_t{ 0x00110101 }));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(scene2, resource->getResourceId()));
        EXPECT_TRUE(m_clientForLoading.impl.getClientApplication().getResource(llhash));
    }

    TEST_F(ResourceDataPoolPersistation, canCloseAResourceFileWhileOtherFileHoldsResourceAndDoesNotLoadLowLevelResource)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");

        Texture2D* resource2 = this->template createResource<Texture2D>("resourceName");
        ASSERT_TRUE(resource != nullptr);
        this->doWriteReadCycle("someTemporaryFile2.ram");
        this->template getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");

        auto llhash = static_cast<Resource*>(resource)->impl.getLowlevelResourceHash();
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash));

        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile2.ram"));
        Scene& scene2(*m_clientForLoading.createScene(sceneId_t{ 0x00110101 }));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(scene2, resource->getResourceId()));
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash));
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(scene2, resource2->getResourceId()));
        auto llhash2 = static_cast<Resource*>(resource2)->impl.getLowlevelResourceHash();
        EXPECT_FALSE(m_clientForLoading.impl.getClientApplication().getResource(llhash2));
    }

    TEST_F(ResourceDataPoolPersistation, failsToRemoveResourceFileItDoesNotHave)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile2.ram"));
    }

    TEST_F(ResourceDataPoolPersistation, failsToRemoveResourceFileTwice)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile.ram"));
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile.ram"));
    }

    TEST_F(ResourceDataPoolPersistation, loadsResourceOnDoubleResourceFileDeletionAndMakesResourceNotInstantiable)
    {
        ArrayResource* resource = this->template createResource<ArrayResource>("resourceName");
        ASSERT_TRUE(resource != nullptr);

        this->doWriteReadCycle("someTemporaryFile.ram");
        this->doWriteReadCycle("someTemporaryFile2.ram");

        // make sure resource is used
        this->template getObjectForTesting<ArrayResource>(resource->getResourceId(), "resourceName");
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).forceLoadResourcesFromResourceDataFile("someTemporaryFile.ram"));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile.ram"));
        EXPECT_TRUE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).removeResourceDataFile("someTemporaryFile2.ram"));

        Scene& newScene(*m_clientForLoading.createScene(sceneId_t{ 0xf00ba2 }));
        EXPECT_TRUE(m_clientForLoading.impl.getClientApplication().getResource(resource->impl.getLowlevelResourceHash()));
        EXPECT_FALSE(RamsesHMIUtils::GetResourceDataPoolForClient(m_clientForLoading).createResourceForScene(newScene, resource->getResourceId()));
    }

    TEST_F(ResourceDataPoolPersistation, doesNotSaveSceneObjectIdsForResourcesButLoadingAssignsNewIds)
    {
        // bump up sceneObjectId counter a bit
        this->template createResource<ArrayResource>("dummy1");
        this->template createResource<ArrayResource>("dummy2");
        this->template createResource<ArrayResource>("dummy3");
        this->template createResource<ArrayResource>("dummy4");

        auto arrayRes = this->template createResource<ArrayResource>("resourceName1");
        ASSERT_TRUE(arrayRes);
        auto texRes = this->template createResource<Texture2D>("resourceName2");
        ASSERT_TRUE(texRes);
        EXPECT_LT(arrayRes->getSceneObjectId().getValue(), texRes->getSceneObjectId().getValue());

        this->doWriteReadCycle("someTemporaryFile.ram");

        auto loadedTexRes = this->template getObjectForTesting<Texture2D>(texRes->getResourceId(), "resourceName2");
        auto loadedArrayRes = this->template getObjectForTesting<ArrayResource>(arrayRes->getResourceId(), "resourceName1");

        EXPECT_NE(loadedTexRes->getSceneObjectId(), sceneObjectId_t::Invalid());
        EXPECT_NE(loadedTexRes->getSceneObjectId(), texRes->getSceneObjectId());
        EXPECT_NE(loadedArrayRes->getSceneObjectId(), sceneObjectId_t::Invalid());
        EXPECT_NE(loadedArrayRes->getSceneObjectId(), arrayRes->getSceneObjectId());
        EXPECT_GT(loadedArrayRes->getSceneObjectId().getValue(), loadedTexRes->getSceneObjectId().getValue());
    }
}
