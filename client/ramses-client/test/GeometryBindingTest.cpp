//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ClientTestUtils.h"
#include "TestEffectCreator.h"

#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/ArrayBuffer.h"

#include "ResourceImpl.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "GeometryBindingImpl.h"
#include "ArrayBufferImpl.h"
#include "Resource/EffectResource.h"
#include "ramses-client-api/EDataType.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/ResourceContentHash.h"

using namespace testing;

namespace ramses
{
    class GeometryBindingTest : public ::testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            sharedTestState = new TestEffectCreator;
        }

        static void TearDownTestCase()
        {
            delete sharedTestState;
            sharedTestState = nullptr;
        }

        void SetUp()
        {
            EXPECT_TRUE(sharedTestState != nullptr);
        }

    protected:
        void checkHashSetToInternalScene(const GeometryBinding& geometryBinding, ramses_internal::DataFieldHandle field, const Resource& resource, ramses_internal::UInt32 expectedInstancingDivisor) const
        {
            const ramses_internal::ResourceContentHash expectedHash = resource.impl.getLowlevelResourceHash();
            const ramses_internal::ResourceField& actualDataResource = sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(), field);
            EXPECT_EQ(expectedHash, actualDataResource.hash);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        void checkDataBufferSetToInternalScene(const GeometryBinding& geometryBinding, ramses_internal::DataFieldHandle field, const ArrayBufferImpl& dataBuffer, ramses_internal::UInt32 expectedInstancingDivisor) const
        {
            const ramses_internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();
            const ramses_internal::ResourceField& actualDataResource = sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(), field);
            EXPECT_EQ(dataBufferHandle, actualDataResource.dataBuffer);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        ArrayResource* setVec3fArrayInput(GeometryBinding& geometry)
        {
            float verts[9] = { 0.f };
            auto vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec3Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        ArrayResource* setVec2fArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.2f };
            auto vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector2F, 4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        ArrayResource* setVec4fArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.4f };
            auto vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector4F, 2u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec4Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec4fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        ArrayResource* setFloatArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.1f };
            auto vertices = sharedTestState->getScene().createArrayResource(EDataType::Float, 8u, verts, ResourceCacheFlag_DoNotCache, "floatVertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        ArrayResource* setIndicesInput(GeometryBinding& geometry)
        {
            uint32_t inds[3] = { 0u };
            auto indices = sharedTestState->getScene().createArrayResource(EDataType::UInt32, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
            EXPECT_TRUE(indices != nullptr);
            assert(indices);

            EXPECT_EQ(0u, geometry.impl.getIndicesCount());
            EXPECT_EQ(StatusOK, geometry.setIndices(*indices));

            return indices;
        }

        static TestEffectCreator* sharedTestState;
    };

    TestEffectCreator* GeometryBindingTest::sharedTestState = nullptr;

    TEST_F(GeometryBindingTest, CanGetEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(sharedTestState->getScene());

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const Effect& resultEffect = geometry->getEffect();
        EXPECT_EQ(resultEffect.getResourceId(), emptyEffect->getResourceId());
        EXPECT_EQ(resultEffect.impl.getLowlevelResourceHash(), emptyEffect->impl.getLowlevelResourceHash());

        const uint32_t fieldCount = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*emptyEffect);
    }

    TEST_F(GeometryBindingTest, dataLayoutHasOnlyIndicesForEmptyEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(sharedTestState->getScene());

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint32_t fieldCount = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*emptyEffect);
    }

    TEST_F(GeometryBindingTest, dataLayoutHasRightEffectHash)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(sharedTestState->getScene());

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const ramses_internal::DataLayout geometryLayout = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout());
        const ramses_internal::ResourceContentHash& effectHashFromGeometryLayout = geometryLayout.getEffectHash();

        EXPECT_EQ(emptyEffect->impl.getLowlevelResourceHash(), effectHashFromGeometryLayout);

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*emptyEffect);
    }

    TEST_F(GeometryBindingTest, indicesFieldIsCreatedAtFixedSlot)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const ramses_internal::DataFieldHandle indicesField(GeometryBindingImpl::IndicesDataFieldIndex);
        const ramses_internal::EFixedSemantics semantics = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getField(indicesField).semantics;
        EXPECT_EQ(ramses_internal::EFixedSemantics::Indices, semantics);

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, canSetResource)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        static const float verts[9] = { 0.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices, 13u));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), *vertices, 13u); // first field is indices

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingResourceToInvalidInput)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        static const float verts[9] = { 0.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingResourceWithMismatchingTypeInEffect)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        static const float verts[8] = { 0.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector2F, 4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getScene().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec2ArrayResourceFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        float verts[8] = { 0.f };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const vertices = anotherScene.createArrayResource(EDataType::Vector2F, 4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec3ArrayResourceFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        float verts[9] = { 0.f };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const vertices = anotherScene.createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec3Vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec4ArrayResourceFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        float verts[8] = { 0.f };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const vertices = anotherScene.createArrayResource(EDataType::Vector4F, 2u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec4fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingIndexDataBufferFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        Scene* otherScene = sharedTestState->getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const indices = otherScene->createArrayBuffer(EDataType::UInt32, 3u, "indices");
        ASSERT_NE(nullptr, indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        sharedTestState->getScene().destroy(*indices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVertexDataBufferFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        Scene* otherScene = sharedTestState->getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const vertices = otherScene->createArrayBuffer(EDataType::Float, 3u, "vertices");
        ASSERT_NE(nullptr, vertices);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*vertices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingArrayBufferByteBlobFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        Scene* otherScene = sharedTestState->getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const vertices = otherScene->createArrayBuffer(EDataType::ByteBlob, 3u, "vertices");
        ASSERT_NE(nullptr, vertices);

        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        constexpr uint16_t nonZeroStride = 13u;
        EXPECT_NE(StatusOK, geometry->setInputBuffer(inputVec2, *vertices, 0u, nonZeroStride));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(inputVec3, *vertices, 2 * sizeof(float), nonZeroStride));

        sharedTestState->getScene().destroy(*vertices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingArrayResourceByteBlobFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        Scene* otherScene = sharedTestState->getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        uint32_t data[4] = { 0u };
        ArrayResource* const vertices = otherScene->createArrayResource(EDataType::ByteBlob, sizeof(data), data);
        ASSERT_NE(nullptr, vertices);

        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        constexpr uint16_t nonZeroStride = 13u;
        EXPECT_NE(StatusOK, geometry->setInputBuffer(inputVec2, *vertices, 0u, nonZeroStride));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(inputVec3, *vertices, 2 * sizeof(float), nonZeroStride));

        sharedTestState->getScene().destroy(*vertices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingFloatArrayResourceFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        float verts[8] = { 0.f };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const vertices = anotherScene.createArrayResource(EDataType::Float, 8u, verts, ResourceCacheFlag_DoNotCache, "floatVertices");
        ASSERT_TRUE(vertices != nullptr);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingUint16ArrayIndicesFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        uint16_t inds[3] = { 0u };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const indices = anotherScene.createArrayResource(EDataType::UInt16, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingUint32ArrayIndicesFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        uint32_t inds[3] = { 0u };
        Scene& anotherScene(*sharedTestState->getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const indices = anotherScene.createArrayResource(EDataType::UInt32, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        sharedTestState->getClient().destroy(anotherScene);
    }

    TEST_F(GeometryBindingTest, canSetIndicesResource16)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint16_t inds[3] = { 0u };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::UInt16, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetIndicesResource32)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint32_t inds[3] = { 0u };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::UInt32, 3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetIndicesDataBuffer16)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const indices = sharedTestState->getScene().createArrayBuffer(EDataType::UInt16, 1u, "index data buffer");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), indices->impl, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetIndicesDataBuffer32)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const indices = sharedTestState->getScene().createArrayBuffer(EDataType::UInt32, 1u, "index data buffer");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), indices->impl, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceFLoat)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::Float, 1u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec2F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::Vector2F, 1u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec3F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 1u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec4F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::Vector4F, 1u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferFloat)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = sharedTestState->getScene().createArrayBuffer(EDataType::Float, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec2F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = sharedTestState->getScene().createArrayBuffer(EDataType::Vector2F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec3F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = sharedTestState->getScene().createArrayBuffer(EDataType::Vector3F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec4F)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = sharedTestState->getScene().createArrayBuffer(EDataType::Vector4F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferByteBlob)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const indices = sharedTestState->getScene().createArrayBuffer(EDataType::ByteBlob, 1u, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceByteBlob)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = sharedTestState->getScene().createArrayResource(EDataType::ByteBlob, sizeof(inds), inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetAttributeResourceInput)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const float verts[9] = { 0.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), *vertices, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, canSetAttributeVertexDataBufferInput)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const vertices = sharedTestState->getScene().createArrayBuffer(EDataType::Vector3F, 3u, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices, 16u));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), vertices->impl, 16u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, canVertexDataBufferInput_ArrayBufferByteBlob)
    {
        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const interleavedVertices = sharedTestState->getScene().createArrayBuffer(EDataType::ByteBlob, 5 * sizeof(float) *3u, "vertices");
        ASSERT_TRUE(interleavedVertices != nullptr);

        constexpr uint16_t nonZeroStride = 17u;
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(2u), interleavedVertices->impl, 0u);
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), interleavedVertices->impl, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*interleavedVertices));
    }

    TEST_F(GeometryBindingTest, canSetVertexDataBufferInput_ArrayResourceByteBlob)
    {
        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        float data[10] = { 1.f };
        ArrayResource* const interleavedVertices = sharedTestState->getScene().createArrayResource(EDataType::ByteBlob, sizeof(data), data);
        ASSERT_TRUE(interleavedVertices != nullptr);

        constexpr uint16_t nonZeroStride = 17u;
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(2u), *interleavedVertices, 0u);
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), *interleavedVertices, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*interleavedVertices));
    }

    TEST_F(GeometryBindingTest, canSetArrayBufferByteBlobToSingleAttribute)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = sharedTestState->getScene().createArrayBuffer(EDataType::ByteBlob, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(2u), vertices->impl, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, canSetArrayResourceByteBlobToSingleAttribute)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        float data[10] = { 1.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::ByteBlob, sizeof(data), data);
        ASSERT_TRUE(vertices);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(2u), *vertices, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, cannotSetStrideAndOffsetToNonByteBlobVertexDataBuffers)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = sharedTestState->getScene().createArrayBuffer(EDataType::Vector2F, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices, 1u, 2u));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, cannotSetStrideAndOffsetToNonByteBlobVertexArrayResource)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        const float verts[2] = { 0.f };
        const auto vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector2F, 1u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices, 1u, 2u));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingAttributeResourceInputWithWrongType)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const float verts[9] = { 0.f };
        ArrayResource* const vertices = sharedTestState->getScene().createArrayResource(EDataType::Vector3F, 3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingAttributeVertexDataBufferInputWithWrongTypeUInt16)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = sharedTestState->getScene().createArrayBuffer(EDataType::UInt16, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingAttributeVertexDataBufferInputWithWrongTypeUInt32)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = sharedTestState->getScene().createArrayBuffer(EDataType::UInt32, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedEffectResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*sharedTestState->effect));
        sharedTestState->effect = nullptr;
        EXPECT_NE(StatusOK, geometry->validate());

        // restore the effect in sharedTestState after this test case
        sharedTestState->effect = sharedTestState->createEffect(sharedTestState->getScene(), false);
        ASSERT_TRUE(nullptr != sharedTestState->effect);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedIndicesResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedInputFloatArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedInputVec2ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedInputVec3ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedInputVec4ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        auto floatArray = setFloatArrayInput(*geometry);
        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedArrayBufferByteBlob)
    {
        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const interleavedVertices = sharedTestState->getScene().createArrayBuffer(EDataType::ByteBlob, 5 * sizeof(float) * 3u, "vertices");
        ASSERT_TRUE(interleavedVertices != nullptr);
        std::vector<ramses_internal::Byte> dummyData(interleavedVertices->getMaximumNumberOfElements(), 0x00);
        interleavedVertices->updateData(0u, 1u, dummyData.data());

        auto floatArray = setFloatArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        constexpr uint16_t nonZeroStride = 12u;
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*interleavedVertices));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedArrayResourceByteBlobl)
    {
        AttributeInput inputVec2;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", inputVec2));
        AttributeInput inputVec3;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", inputVec3));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        uint32_t data[4] = { 0u };
        ArrayResource* const interleavedVertices = sharedTestState->getScene().createArrayResource(EDataType::ByteBlob, sizeof(data), data);
        ASSERT_TRUE(interleavedVertices != nullptr);

        auto floatArray = setFloatArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        constexpr uint16_t nonZeroStride = 12u;
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(inputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*interleavedVertices));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedVertexDataBuffer)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");

        ArrayBuffer* const vertexDataBuffer = sharedTestState->getScene().createArrayBuffer(EDataType::Float, 3, "vertices");
        ASSERT_TRUE(vertexDataBuffer != nullptr);
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        geometry->setInputBuffer(input, *vertexDataBuffer);
        const float data[] = { 0 };
        vertexDataBuffer->updateData(0u, 1u, data);

        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertexDataBuffer));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithVertexDataBufferThatHasWrongType)
    {
        //It is possible that a data buffer gets deleted, and new data buffer gets created with same
        //handle. Unfortunately validate can not check if a data buffer was destroyed and re-created
        //but it can at least check that the assigned data buffer is of a correct type
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");

        ArrayBuffer* const vertexDataBuffer = sharedTestState->getScene().createArrayBuffer(EDataType::Float, 3, "vertices");
        ASSERT_TRUE(vertexDataBuffer != nullptr);
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        geometry->setInputBuffer(input, *vertexDataBuffer);
        const float data[] = { 0 };
        vertexDataBuffer->updateData(0u, 1u, data);

        auto vec2fArray = setVec2fArrayInput(*geometry);
        auto vec3fArray = setVec3fArrayInput(*geometry);
        auto vec4fArray = setVec4fArrayInput(*geometry);
        auto indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        //delete data buffer and create new one with same handle
        ramses_internal::DataBufferHandle dataBufferHandle = vertexDataBuffer->impl.getDataBufferHandle();
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertexDataBuffer));
        ASSERT_FALSE(sharedTestState->getScene().impl.getIScene().isDataBufferAllocated(dataBufferHandle));
        sharedTestState->getScene().impl.getIScene().allocateDataBuffer(ramses_internal::EDataBufferType::VertexBuffer, ramses_internal::EDataType::Vector2F, 10 * sizeof(float), dataBufferHandle);
        ASSERT_TRUE(sharedTestState->getScene().impl.getIScene().isDataBufferAllocated(dataBufferHandle));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indicesArray));
    }
}
