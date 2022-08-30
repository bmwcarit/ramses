//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-client-api/ResourceDataPool.h"
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/Resource.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/Texture2D.h"
#include "ramses-client-api/Texture3D.h"
#include "ramses-client-api/TextureCube.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/Effect.h"
#include "ramses-hmi-utils.h"

#include "ResourceDataPoolImpl.h"
#include "gtest/gtest.h"
#include "CreationHelper.h"
#include "RamsesObjectTypeTraits.h"
#include "RamsesObjectTestTypes.h"
#include "RamsesClientImpl.h"
#include "ResourceImpl.h"

namespace ramses
{
    class AResourceDataPool : public testing::Test
    {
    public:
        AResourceDataPool()
            : framework()
            , client(*framework.createClient(""))
            , scene(*client.createScene(sceneId_t{ 0xf00 }))
            , otherScene(*client.createScene(sceneId_t{ 0xf00b42 }))
            , rdp(RamsesHMIUtils::GetResourceDataPoolForClient(client))
            , helper(&scene, nullptr, &client)
        {
            brokenEffectDesc.setVertexShader("#version 100\n"
                "attribute float inp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = vec4(0.0)\n"
                "}\n");
            brokenEffectDesc.setFragmentShader("precision highp float;"
                "void main(void)\n"
                "{"
                "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
                "}");
            workingEffectDesc.setVertexShader("#version 100\n"
                "attribute float inp;\n"
                "void main(void)\n"
                "{\n"
                "    gl_Position = vec4(0.0)\n;"
                "}\n");
            workingEffectDesc.setFragmentShader("precision highp float;"
                "void main(void)\n"
                "{"
                "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
                "}");
        }

        template <class ResourceType>
        resourceId_t createResourceData()
        {
            switch (TYPE_ID_OF_RAMSES_OBJECT<ResourceType>::ID)
            {
            case ERamsesObjectType_ArrayResource:
                return createArrayResourceData();
            case ERamsesObjectType_Texture2D:
                return createTexture2DData();
            case ERamsesObjectType_Texture3D:
                return createTexture3DData();
            case ERamsesObjectType_TextureCube:
                return createTextureCubeData();
            case ERamsesObjectType_Effect:
                return createEffectData();
            default:
                return {};
            }
        }

        resourceId_t createArrayResourceData()
        {
            uint16_t data = 0;
            return rdp.addArrayResourceData(EDataType::UInt16, 1, &data, ResourceCacheFlag_DoNotCache, resName);
        }

        resourceId_t createTexture2DData()
        {
            uint8_t data[4] = { 0u };
            MipLevelData mipLevelData(sizeof(data), data);
            TextureSwizzle swizzle;
            return rdp.addTexture2DData(ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, resName);
        }

        resourceId_t createTexture3DData()
        {
            uint8_t data[32] = { 0u };
            MipLevelData mipLevelData(sizeof(data), data);
            return rdp.addTexture3DData(ETextureFormat::RGBA8, 1u, 2u, 4u, 1, &mipLevelData, false, ResourceCacheFlag_DoNotCache, resName);
        }

        resourceId_t createTextureCubeData()
        {
            uint8_t data[4] = { 0u };
            CubeMipLevelData mipLevelData(sizeof(data), data, data, data, data, data, data);
            TextureSwizzle swizzle;
            return rdp.addTextureCubeData(ETextureFormat::RGBA8, 1u, 1, &mipLevelData, false, swizzle, ResourceCacheFlag_DoNotCache, resName);
        }

        resourceId_t createEffectData()
        {
            EffectDescription effectDescription;
            effectDescription.setVertexShader("void main(void) {gl_Position=vec4(0);}");
            effectDescription.setFragmentShader("void main(void) {gl_FragColor=vec4(1);}");
            return rdp.addEffectData(effectDescription, ResourceCacheFlag_DoNotCache, resName);
        }

        template <class ObjectType>
        ObjectType* instantiateVerifyAndCastResource(resourceId_t const& id, bool useOtherScene = false)
        {
            auto& usedScene = useOtherScene ? otherScene : scene;
            EXPECT_NE(id, resourceId_t::Invalid());
            auto res = rdp.createResourceForScene(usedScene, id);

            EXPECT_TRUE(res);
            EXPECT_EQ(res, usedScene.getResource(id));
            if (resName)
            {
                EXPECT_EQ(res, usedScene.findObjectByName(resName));
                EXPECT_STREQ(res->getName(), resName);
            }
            else
                EXPECT_STREQ(res->getName(), "");

            EXPECT_EQ(res->getResourceId(), id);
            auto type = TYPE_ID_OF_RAMSES_OBJECT<ObjectType>::ID;
            EXPECT_EQ(res->getType(), type);
            EXPECT_TRUE(client.impl.getClientApplication().getResource(res->impl.getLowlevelResourceHash()));

            return static_cast<ObjectType*>(res);
        }

        RamsesFramework framework;
        RamsesClient& client;
        Scene& scene;
        Scene& otherScene;

        ResourceDataPool& rdp;

        CreationHelper helper;

        const char* resName = "myResource";

        EffectDescription brokenEffectDesc;
        EffectDescription workingEffectDesc;
    };

    template <class ResourceType>
    class AResourceDataPoolTyped : public AResourceDataPool
    {
    public:
    };

    TYPED_TEST_SUITE(AResourceDataPoolTyped, ResourceTypes);

    TEST_F(AResourceDataPool, createResourceForSceneReturnsNullptrWithInvalidId)
    {
        EXPECT_FALSE(rdp.createResourceForScene(scene, {}));
    }

    TEST_F(AResourceDataPool, createResourceForSceneReturnsNullptrWithNonExistingId)
    {
        EXPECT_FALSE(rdp.createResourceForScene(scene, {0xf00, 0xba2}));
    }

    TEST_F(AResourceDataPool, canCreateArrayResourceDataAndInstantiateItForScene)
    {
        auto id = createArrayResourceData();
        auto res = instantiateVerifyAndCastResource<ArrayResource>(id);

        EXPECT_EQ(res->getDataType(), EDataType::UInt16);
        EXPECT_EQ(res->getNumberOfElements(), 1u);
    }

    TEST_F(AResourceDataPool, canCreateAnotherKindOfArrayResourceDataAndInstantiateItForScene)
    {
        float data[4] = { .0f, .0f, .0f, .0f };
        auto id = rdp.addArrayResourceData(EDataType::Vector2F, 2, &data, ResourceCacheFlag_DoNotCache, resName);
        auto res = instantiateVerifyAndCastResource<ArrayResource>(id);

        EXPECT_EQ(res->getDataType(), EDataType::Vector2F);
        EXPECT_EQ(res->getNumberOfElements(), 2u);
    }

    TEST_F(AResourceDataPool, canCreateTexture2DDataAndInstantiateItForScene)
    {
        auto id = createTexture2DData();
        auto res = instantiateVerifyAndCastResource<Texture2D>(id);

        EXPECT_EQ(res->getWidth(), 1u);
        EXPECT_EQ(res->getHeight(), 1u);
        EXPECT_EQ(res->getTextureFormat(), ETextureFormat::RGBA8);
        TextureSwizzle swizzle;
        EXPECT_EQ(res->getTextureSwizzle().channelRed, swizzle.channelRed);
        EXPECT_EQ(res->getTextureSwizzle().channelGreen, swizzle.channelGreen);
        EXPECT_EQ(res->getTextureSwizzle().channelBlue, swizzle.channelBlue);
        EXPECT_EQ(res->getTextureSwizzle().channelAlpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceDataPool, canCreateTexture3DDataAndInstantiateItForScene)
    {
        auto id = createTexture3DData();
        auto res = instantiateVerifyAndCastResource<Texture3D>(id);

        EXPECT_EQ(res->getWidth(), 1u);
        EXPECT_EQ(res->getHeight(), 2u);
        EXPECT_EQ(res->getDepth(), 4u);
        EXPECT_EQ(res->getTextureFormat(), ETextureFormat::RGBA8);
    }

    TEST_F(AResourceDataPool, canCreateTextureCubeDataAndInstantiateItForScene)
    {
        auto id = createTextureCubeData();
        auto res = instantiateVerifyAndCastResource<TextureCube>(id);

        EXPECT_EQ(res->getSize(), 1u);
        EXPECT_EQ(res->getTextureFormat(), ETextureFormat::RGBA8);
        TextureSwizzle swizzle;
        EXPECT_EQ(res->getTextureSwizzle().channelRed, swizzle.channelRed);
        EXPECT_EQ(res->getTextureSwizzle().channelGreen, swizzle.channelGreen);
        EXPECT_EQ(res->getTextureSwizzle().channelBlue, swizzle.channelBlue);
        EXPECT_EQ(res->getTextureSwizzle().channelAlpha, swizzle.channelAlpha);
    }

    TEST_F(AResourceDataPool, canCreateEffectDataAndInstantiateItForScene)
    {
        auto id = createEffectData();
        auto res = instantiateVerifyAndCastResource<Effect>(id);

        EXPECT_EQ(res->getUniformInputCount(), 0u);
        EXPECT_EQ(res->getAttributeInputCount(), 0u);

        EXPECT_EQ(rdp.getLastEffectErrorMessages(), "");
    }

    TEST_F(AResourceDataPool, willReportFailuresOfEffectCreationIntoEffectErrorMessagesAndResetIt)
    {
        EXPECT_EQ(rdp.addEffectData(brokenEffectDesc), resourceId_t::Invalid());
        EXPECT_NE(rdp.getLastEffectErrorMessages(), "");
        EXPECT_NE(rdp.addEffectData(workingEffectDesc), resourceId_t::Invalid());
        EXPECT_EQ(rdp.getLastEffectErrorMessages(), "");
    }

    TEST_F(AResourceDataPool, doesNotMixUpSceneEffectErrorMessagesAndHisOwn)
    {
        EXPECT_NE(rdp.addEffectData(workingEffectDesc), resourceId_t::Invalid());
        EXPECT_FALSE(scene.createEffect(brokenEffectDesc));
        EXPECT_EQ(rdp.getLastEffectErrorMessages(), "");
        EXPECT_NE(rdp.addEffectData(workingEffectDesc), resourceId_t::Invalid());
        EXPECT_NE(scene.getLastEffectErrorMessages(), "");
    }

    TEST_F(AResourceDataPool, doesNotMixUpSceneEffectErrorMessagesAndHisOwnViceVersa)
    {
        EXPECT_TRUE(scene.createEffect(workingEffectDesc));
        EXPECT_EQ(rdp.addEffectData(brokenEffectDesc), resourceId_t::Invalid());
        EXPECT_EQ(scene.getLastEffectErrorMessages(), "");
        EXPECT_TRUE(scene.createEffect(workingEffectDesc));
        EXPECT_NE(rdp.getLastEffectErrorMessages(), "");
    }

    TYPED_TEST(AResourceDataPoolTyped, acceptsNullptrAsName)
    {
        this->resName = nullptr;
        auto id = this->template createResourceData<TypeParam>();

        this->template instantiateVerifyAndCastResource<TypeParam>(id);
    }

    TYPED_TEST(AResourceDataPoolTyped, canCreateArrayResourceDataTwiceWithSameParametersWillResultInSameID)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto id2 = this->template createResourceData<TypeParam>();
        EXPECT_EQ(id, id2);

        this->template instantiateVerifyAndCastResource<TypeParam>(id2);
    }

    TYPED_TEST(AResourceDataPoolTyped, cantCreatePoolResourceFromSceneCreatedResourceID)
    {
        auto sceneResource = this->helper.template createObjectOfType<TypeParam>(this->resName);
        EXPECT_FALSE(this->rdp.createResourceForScene(this->scene, sceneResource->getResourceId()));
        EXPECT_FALSE(this->rdp.createResourceForScene(this->otherScene, sceneResource->getResourceId()));
    }

    TYPED_TEST(AResourceDataPoolTyped, canCreateSceneCreatedObjectAndIdenticalPoolResourceAndThenInstantiateItInAnotherScene)
    {
        auto sceneResource = this->helper.template createObjectOfType<TypeParam>(this->resName);
        auto id = this->template createResourceData<TypeParam>();
        EXPECT_EQ(sceneResource->getResourceId(), id);
        EXPECT_FALSE(this->rdp.createResourceForScene(this->scene, id));
        EXPECT_TRUE(this->rdp.createResourceForScene(this->otherScene, id));
    }

    TYPED_TEST(AResourceDataPoolTyped, canCreateResourceTwiceForDifferentScenesFromOnePoolData)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto res1 = this->template instantiateVerifyAndCastResource<TypeParam>(id, false);
        auto res2 = this->template instantiateVerifyAndCastResource<TypeParam>(id, true);
        EXPECT_NE(res1, res2);
        EXPECT_EQ(res1->getResourceId(), res2->getResourceId());
        EXPECT_EQ(res1->getType(), res2->getType());
        EXPECT_STREQ(res1->getName(), res2->getName());
    }

    TYPED_TEST(AResourceDataPoolTyped, canRecreateResourceAgainAfterDeletingFromScene)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto res = this->template instantiateVerifyAndCastResource<TypeParam>(id);
        this->scene.destroy(*res);
        this->template instantiateVerifyAndCastResource<TypeParam>(id);
    }

    TEST_F(AResourceDataPool, failsWhenTryingToRemoveUnknownResource)
    {
        EXPECT_FALSE(this->rdp.removeResourceData({ 0xf00, 0xba2 }));
    }

    TYPED_TEST(AResourceDataPoolTyped, canAddAndThenRemoveFromPoolFailsToCreateResourceAfterwards)
    {
        auto id = this->template createResourceData<TypeParam>();
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.createResourceForScene(this->scene, id));
    }

    TYPED_TEST(AResourceDataPoolTyped, canAddAndThenRemoveFromPoolFailsToRemoveTwice)
    {
        auto id = this->template createResourceData<TypeParam>();
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.removeResourceData(id));
    }

    TYPED_TEST(AResourceDataPoolTyped, canAddAndInstantiateSceneKeepsLLResourceAlive)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto res = this->template instantiateVerifyAndCastResource<TypeParam>(id);
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.createResourceForScene(this->scene, id));

        EXPECT_EQ(res->getResourceId(), id);
        auto llhash = static_cast<Resource*>(res)->impl.getLowlevelResourceHash();
        EXPECT_TRUE(this->client.impl.getClientApplication().getResource(llhash));
        this->scene.destroy(*res);
        EXPECT_FALSE(this->client.impl.getClientApplication().getResource(llhash));
    }

    TYPED_TEST(AResourceDataPoolTyped, canAddAndInstantiatePoolKeepsLLResourceAlive)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto res = this->template instantiateVerifyAndCastResource<TypeParam>(id);
        auto llhash = static_cast<Resource*>(res)->impl.getLowlevelResourceHash();
        this->scene.destroy(*res);

        EXPECT_TRUE(this->client.impl.getClientApplication().getResource(llhash));
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->client.impl.getClientApplication().getResource(llhash));
    }

    TYPED_TEST(AResourceDataPoolTyped, canAddResourceMultipleTimesAndThenAllowsRemovingExactlyAsOften)
    {
        auto id = this->template createResourceData<TypeParam>();
        auto id2 = this->template createResourceData<TypeParam>();
        ASSERT_EQ(id, id2);
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.removeResourceData(id));

        this->template createResourceData<TypeParam>();
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.removeResourceData(id));

        this->template createResourceData<TypeParam>();
        this->template createResourceData<TypeParam>();
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        this->template createResourceData<TypeParam>();
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_TRUE(this->rdp.removeResourceData(id));
        EXPECT_FALSE(this->rdp.removeResourceData(id));
    }
}
