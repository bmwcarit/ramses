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
#include "ramses-client-api/FloatArray.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/UInt32Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/IndexDataBuffer.h"
#include "ramses-client-api/VertexDataBuffer.h"

#include "ResourceImpl.h"
#include "EffectImpl.h"
#include "ArrayResourceImpl.h"
#include "GeometryBindingImpl.h"
#include "IndexDataBufferImpl.h"
#include "VertexDataBufferImpl.h"
#include "Resource/EffectResource.h"
#include "ramses-client-api/EDataType.h"
#include "SceneAPI/EDataType.h"

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
            sharedTestState = NULL;
        }

        void SetUp()
        {
            EXPECT_TRUE(sharedTestState != NULL);
        }

    protected:
        void checkHashSetToInternalScene(const GeometryBinding& geometryBinding, ramses_internal::DataFieldHandle field, const Resource& resource, ramses_internal::UInt32 expectedInstancingDivisor) const
        {
            const ramses_internal::ResourceContentHash expectedHash = resource.impl.getLowlevelResourceHash();
            const ramses_internal::ResourceField& actualDataResource = sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(), field);
            EXPECT_EQ(expectedHash, actualDataResource.hash);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        void checkHashNotSetInInternalScene(const GeometryBinding& geometryBinding, ramses_internal::DataFieldHandle field) const
        {
            const ramses_internal::ResourceField& actualDataResource = sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(), field);
            EXPECT_EQ(ramses_internal::ResourceContentHash::Invalid(), actualDataResource.hash);
        }

        void checkDataBufferSetToInternalScene(const GeometryBinding& geometryBinding, ramses_internal::DataFieldHandle field, const DataBufferImpl& dataBuffer, ramses_internal::UInt32 expectedInstancingDivisor) const
        {
            const ramses_internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();
            const ramses_internal::ResourceField& actualDataResource = sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(), field);
            EXPECT_EQ(dataBufferHandle, actualDataResource.dataBuffer);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        void checkDataBufferNotSetInInternalScene(const GeometryBinding&           geometryBinding,
                                               ramses_internal::DataFieldHandle field) const
        {
            const ramses_internal::ResourceField&   actualDataResource =
                sharedTestState->getInternalScene().getDataResource(geometryBinding.impl.getAttributeDataInstance(),
                                                                    field);
            EXPECT_EQ(ramses_internal::DataBufferHandle::Invalid(), actualDataResource.dataBuffer);
        }

        const Vector3fArray* setVec3fArrayInput(GeometryBinding& geometry)
        {
            float verts[9] = { 0.f };
            const Vector3fArray* vertices = sharedTestState->getClient().createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec3Vertices");
            EXPECT_TRUE(vertices != NULL);
            if (!vertices)
                return nullptr;

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        const Vector2fArray* setVec2fArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.2f };
            const Vector2fArray* vertices = sharedTestState->getClient().createConstVector2fArray(4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
            EXPECT_TRUE(vertices != NULL);
            if (!vertices)
                return nullptr;

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        const Vector4fArray* setVec4fArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.4f };
            const Vector4fArray* vertices = sharedTestState->getClient().createConstVector4fArray(2u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec4Vertices");
            EXPECT_TRUE(vertices != NULL);
            if (!vertices)
                return nullptr;

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec4fArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        const FloatArray* setFloatArrayInput(GeometryBinding& geometry)
        {
            float verts[8] = { 0.1f };
            const FloatArray* vertices = sharedTestState->getClient().createConstFloatArray(8u, verts, ResourceCacheFlag_DoNotCache, "floatVertices");
            EXPECT_TRUE(vertices != NULL);
            if (!vertices)
                return nullptr;

            AttributeInput input;
            EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
            EXPECT_EQ(StatusOK, geometry.setInputBuffer(input, *vertices));

            return vertices;
        }

        const UInt32Array* setIndicesInput(GeometryBinding& geometry)
        {
            uint32_t inds[3] = { 0u };
            const UInt32Array* indices = sharedTestState->getClient().createConstUInt32Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
            EXPECT_TRUE(indices != NULL);
            if (!indices)
                return nullptr;

            EXPECT_EQ(0u, geometry.impl.getIndicesCount());
            EXPECT_EQ(StatusOK, geometry.setIndices(*indices));

            return indices;
        }

        static TestEffectCreator* sharedTestState;
    };

    TestEffectCreator* GeometryBindingTest::sharedTestState = 0;

    TEST_F(GeometryBindingTest, CanGetEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(sharedTestState->getClient());

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const Effect& resultEffect = geometry->getEffect();
        EXPECT_EQ(resultEffect.getResourceId(), emptyEffect->getResourceId());
        EXPECT_EQ(resultEffect.impl.getLowlevelResourceHash(), emptyEffect->impl.getLowlevelResourceHash());

        const uint32_t fieldCount = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*emptyEffect);
    }

    TEST_F(GeometryBindingTest, dataLayoutHasOnlyIndicesForEmptyEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(sharedTestState->getClient());

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const uint32_t fieldCount = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*emptyEffect);
    }

    TEST_F(GeometryBindingTest, indicesFieldIsCreatedAtFixedSlot)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const ramses_internal::DataFieldHandle indicesField(GeometryBindingImpl::IndicesDataFieldIndex);
        const ramses_internal::EFixedSemantics semantics = sharedTestState->getInternalScene().getDataLayout(geometry->impl.getAttributeDataLayout()).getField(indicesField).semantics;
        EXPECT_EQ(ramses_internal::EFixedSemantics_Indices, semantics);

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, canSetResource)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        static const float verts[9] = { 0.f };
        const Vector3fArray* const vertices = sharedTestState->getClient().createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices, 13u));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), *vertices, 13u); // first field is indices

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingResourceToInvalidInput)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        static const float verts[9] = { 0.f };
        const Vector3fArray* const vertices = sharedTestState->getClient().createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingResourceWithMismatchingTypeInEffect)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        static const float verts[8] = { 0.f };
        const Vector2fArray* const vertices = sharedTestState->getClient().createConstVector2fArray(4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*vertices);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec2ArrayResourceFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        float verts[8] = { 0.f };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const Vector2fArray* const vertices = anotherClient.createConstVector2fArray(4u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec3ArrayResourceFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        float verts[9] = { 0.f };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const Vector3fArray* const vertices = anotherClient.createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec3Vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVec4ArrayResourceFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        float verts[8] = { 0.f };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const Vector4fArray* const vertices = anotherClient.createConstVector4fArray(2u, verts, ramses::ResourceCacheFlag_DoNotCache, "vec2Vertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec4fArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingIndexDataBufferFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        Scene* otherScene = sharedTestState->getClient().createScene(777u);
        ASSERT_NE(nullptr, otherScene);

        IndexDataBuffer* const indices = otherScene->createIndexDataBuffer(3 * sizeof(uint32_t), EDataType_UInt32, "indices");
        ASSERT_NE(nullptr, indices);

        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        sharedTestState->getScene().destroy(*indices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingVertexDataBufferFromAnotherScene)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        Scene* otherScene = sharedTestState->getClient().createScene(777u);
        ASSERT_NE(nullptr, otherScene);

        VertexDataBuffer* const vertices = otherScene->createVertexDataBuffer(3 * sizeof(float), EDataType_Float, "vertices");
        ASSERT_NE(nullptr, vertices);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*vertices);
        sharedTestState->getScene().destroy(*geometry);
        sharedTestState->getClient().destroy(*otherScene);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingFloatArrayResourceFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        float verts[8] = { 0.f };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const FloatArray* const vertices = anotherClient.createConstFloatArray(8u, verts, ResourceCacheFlag_DoNotCache, "floatVertices");
        ASSERT_TRUE(vertices != NULL);

        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        sharedTestState->getScene().destroy(*geometry);
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingUint16ArrayIndicesFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        uint16_t inds[3] = { 0u };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const UInt16Array* const indices = anotherClient.createConstUInt16Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != NULL);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingUint32ArrayIndicesFromAnotherClient)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        uint32_t inds[3] = { 0u };
        RamsesClient anotherClient("anotherLocalTestClient", sharedTestState->getFramework());
        const UInt32Array* const indices = anotherClient.createConstUInt32Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != NULL);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_NE(StatusOK, geometry->setIndices(*indices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
    }

    TEST_F(GeometryBindingTest, canSetIndicesResource16)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const uint16_t inds[3] = { 0u };
        const UInt16Array* const indices = sharedTestState->getClient().createConstUInt16Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != NULL);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetIndicesResource32)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const uint32_t inds[3] = { 0u };
        const UInt32Array* const indices = sharedTestState->getClient().createConstUInt32Array(3u, inds, ramses::ResourceCacheFlag_DoNotCache, "indices");
        ASSERT_TRUE(indices != NULL);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetIndicesDataBuffer16)
    {
        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        IndexDataBuffer* const indices = sharedTestState->getScene().createIndexDataBuffer(sizeof(uint16_t), EDataType_UInt16, "index data buffer");
        ASSERT_TRUE(indices != NULL);

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
        ASSERT_TRUE(geometry != NULL);

        IndexDataBuffer* const indices = sharedTestState->getScene().createIndexDataBuffer(sizeof(uint32_t), EDataType_UInt32, "index data buffer");
        ASSERT_TRUE(indices != NULL);

        EXPECT_EQ(0u, geometry->impl.getIndicesCount());
        EXPECT_EQ(StatusOK, geometry->setIndices(*indices));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(0u), indices->impl, 0u);
        EXPECT_EQ(indices->impl.getElementCount(), geometry->impl.getIndicesCount());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*indices));
    }

    TEST_F(GeometryBindingTest, canSetAttributeResourceInput)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const float verts[9] = { 0.f };
        const Vector3fArray* const vertices = sharedTestState->getClient().createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != NULL);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices));
        checkHashSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), *vertices, 0u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, canSetAttributeVertexDataBufferInput)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec3fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        VertexDataBuffer* const vertices = sharedTestState->getScene().createVertexDataBuffer(3 * sizeof(float), EDataType_Vector3F, "vertices");
        ASSERT_TRUE(vertices != NULL);

        EXPECT_EQ(StatusOK, geometry->setInputBuffer(input, *vertices, 16u));
        checkDataBufferSetToInternalScene(*geometry, ramses_internal::DataFieldHandle(3u), vertices->impl, 16u);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingAttributeResourceInputWithWrongType)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        const float verts[9] = { 0.f };
        const Vector3fArray* const vertices = sharedTestState->getClient().createConstVector3fArray(3u, verts, ramses::ResourceCacheFlag_DoNotCache, "vertices");
        ASSERT_TRUE(vertices != NULL);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenSettingAttributeVertexDataBufferInputWithWrongType)
    {
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("vec2fArrayInput", input));

        GeometryBinding* const geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        ASSERT_TRUE(geometry != NULL);

        VertexDataBuffer* const vertices = sharedTestState->getScene().createVertexDataBuffer(sizeof(float), EDataType_Float, "vertices");
        ASSERT_TRUE(vertices != NULL);

        EXPECT_NE(StatusOK, geometry->setInputBuffer(input, *vertices));

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertices));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidEffectReference)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*sharedTestState->effect));
        sharedTestState->effect = NULL;
        EXPECT_NE(StatusOK, geometry->validate());

        // restore the effect in sharedTestState after this test case
        sharedTestState->effect = sharedTestState->createEffect(sharedTestState->getClient(), false);
        ASSERT_TRUE(NULL != sharedTestState->effect);

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidIndicesReference)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidInputFloatArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidInputVec2ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidInputVec3ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithInvalidInputVec4ArrayResource)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");
        const FloatArray* floatArray = setFloatArrayInput(*geometry);
        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*floatArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithDestroyedVertexDataBuffer)
    {
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");

        VertexDataBuffer* const vertexDataBuffer = sharedTestState->getScene().createVertexDataBuffer(3 * sizeof(float), EDataType_Float, "vertices");
        ASSERT_TRUE(vertexDataBuffer != NULL);
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        geometry->setInputBuffer(input, *vertexDataBuffer);
        const char data[] = { 0 };
        vertexDataBuffer->setData(data, 1u);

        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertexDataBuffer));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }

    TEST_F(GeometryBindingTest, reportsErrorWhenValidatedWithVertexDataBufferThatHasWrongType)
    {
        //It is possible that a data buffer gets deleted, and new data buffer gets created with same
        //handle. Unfortunately validate can not check if a data buffer was destroyed and re-created
        //but it can at least check that the assigned data buffer is of a correct type
        GeometryBinding* geometry = sharedTestState->getScene().createGeometryBinding(*sharedTestState->effect, "geometry");

        VertexDataBuffer* const vertexDataBuffer = sharedTestState->getScene().createVertexDataBuffer(3 * sizeof(float), EDataType_Float, "vertices");
        ASSERT_TRUE(vertexDataBuffer != NULL);
        AttributeInput input;
        EXPECT_EQ(StatusOK, sharedTestState->effect->findAttributeInput("floatArrayInput", input));
        geometry->setInputBuffer(input, *vertexDataBuffer);
        const char data[] = { 0 };
        vertexDataBuffer->setData(data, 1u);

        const Vector2fArray* vec2fArray = setVec2fArrayInput(*geometry);
        const Vector3fArray* vec3fArray = setVec3fArrayInput(*geometry);
        const Vector4fArray* vec4fArray = setVec4fArrayInput(*geometry);
        const UInt32Array* indicesArray = setIndicesInput(*geometry);
        EXPECT_EQ(StatusOK, geometry->validate());

        //delete data buffer and create new one with same handle
        ramses_internal::DataBufferHandle dataBufferHandle = vertexDataBuffer->impl.getDataBufferHandle();
        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*vertexDataBuffer));
        ASSERT_FALSE(sharedTestState->getScene().impl.getIScene().isDataBufferAllocated(dataBufferHandle));
        sharedTestState->getScene().impl.getIScene().allocateDataBuffer(ramses_internal::EDataBufferType::VertexBuffer, ramses_internal::EDataType_Vector2F, 10 * sizeof(float), dataBufferHandle);
        ASSERT_TRUE(sharedTestState->getScene().impl.getIScene().isDataBufferAllocated(dataBufferHandle));
        EXPECT_NE(StatusOK, geometry->validate());

        EXPECT_EQ(StatusOK, sharedTestState->getScene().destroy(*geometry));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec2fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec3fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*vec4fArray));
        EXPECT_EQ(StatusOK, sharedTestState->getClient().destroy(*indicesArray));
    }
}
