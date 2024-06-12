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
#include "FeatureLevelTestValues.h"

#include "ramses/client/Geometry.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/ArrayBuffer.h"

#include "impl/ResourceImpl.h"
#include "impl/EffectImpl.h"
#include "impl/ArrayResourceImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "ramses/framework/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

using namespace testing;

namespace ramses::internal
{
    class GeometryTest : public TestWithSharedEffectPerFeatureLevel
    {
    protected:
        void CheckHashSetToInternalScene(const Geometry& geometryBinding, ramses::internal::DataFieldHandle field, const Resource& resource, uint32_t expectedInstancingDivisor)
        {
            const ramses::internal::ResourceContentHash expectedHash = resource.impl().getLowlevelResourceHash();
            const ramses::internal::ResourceField& actualDataResource = m_sharedTestState.getInternalScene().getDataResource(geometryBinding.impl().getAttributeDataInstance(), field);
            EXPECT_EQ(expectedHash, actualDataResource.hash);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        void CheckDataBufferSetToInternalScene(const Geometry& geometryBinding, ramses::internal::DataFieldHandle field, const ArrayBufferImpl& dataBuffer, uint32_t expectedInstancingDivisor)
        {
            const ramses::internal::DataBufferHandle dataBufferHandle = dataBuffer.getDataBufferHandle();
            const ramses::internal::ResourceField& actualDataResource = m_sharedTestState.getInternalScene().getDataResource(geometryBinding.impl().getAttributeDataInstance(), field);
            EXPECT_EQ(dataBufferHandle, actualDataResource.dataBuffer);
            EXPECT_EQ(expectedInstancingDivisor, actualDataResource.instancingDivisor);
        }

        ArrayResource* SetVec3fArrayInput(Geometry& geometry)
        {
            const vec3f vert{ 0.f, 1.f, 2.f };
            auto vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vec3Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            const auto optInput = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
            EXPECT_TRUE(optInput.has_value());
            assert(optInput != std::nullopt);
            EXPECT_TRUE(geometry.setInputBuffer(*optInput, *vertices));

            return vertices;
        }

        ArrayResource* SetVec2fArrayInput(Geometry& geometry)
        {
            const vec2f vert{ 0.f, 1.f };
            auto vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vec2Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
            EXPECT_TRUE(optInput.has_value());
            assert(optInput != std::nullopt);
            EXPECT_TRUE(geometry.setInputBuffer(*optInput, *vertices));

            return vertices;
        }

        ArrayResource* SetVec4fArrayInput(Geometry& geometry)
        {
            const vec4f vert{ 0.f, 1.f, 2.f, 3.f };
            auto vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vec4Vertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            const auto optInput = m_sharedTestState.effect->findAttributeInput("vec4fArrayInput");
            EXPECT_TRUE(optInput.has_value());
            assert(optInput != std::nullopt);
            EXPECT_TRUE(geometry.setInputBuffer(*optInput, *vertices));

            return vertices;
        }

        ArrayResource* SetFloatArrayInput(Geometry& geometry)
        {
            float verts[8] = { 0.1f };
            auto vertices = m_sharedTestState.getScene().createArrayResource(8u, verts, "floatVertices");
            EXPECT_TRUE(vertices != nullptr);
            assert(vertices);

            const auto optInput = m_sharedTestState.effect->findAttributeInput("floatArrayInput");
            EXPECT_TRUE(optInput.has_value());
            assert(optInput != std::nullopt);
            EXPECT_TRUE(geometry.setInputBuffer(*optInput, *vertices));

            return vertices;
        }

        ArrayResource* SetIndicesInput(Geometry& geometry)
        {
            uint32_t inds[3] = { 0u };
            auto indices = m_sharedTestState.getScene().createArrayResource(3u, inds, "indices");
            EXPECT_TRUE(indices != nullptr);
            assert(indices);

            EXPECT_EQ(0u, geometry.impl().getIndicesCount());
            EXPECT_TRUE(geometry.setIndices(*indices));

            return indices;
        }
    };

    RAMSES_INSTANTIATE_FEATURELEVEL_TEST_SUITE(GeometryTest);

    TEST_P(GeometryTest, CanGetEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(m_sharedTestState.getScene());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const Effect& resultEffect = geometry->getEffect();
        EXPECT_EQ(resultEffect.getResourceId(), emptyEffect->getResourceId());
        EXPECT_EQ(resultEffect.impl().getLowlevelResourceHash(), emptyEffect->impl().getLowlevelResourceHash());

        const uint32_t fieldCount = m_sharedTestState.getInternalScene().getDataLayout(geometry->impl().getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getScene().destroy(*emptyEffect);
    }

    TEST_P(GeometryTest, dataLayoutHasOnlyIndicesForEmptyEffect)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(m_sharedTestState.getScene());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint32_t fieldCount = m_sharedTestState.getInternalScene().getDataLayout(geometry->impl().getAttributeDataLayout()).getFieldCount();
        EXPECT_EQ(1u, fieldCount);

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getScene().destroy(*emptyEffect);
    }

    TEST_P(GeometryTest, dataLayoutHasRightEffectHash)
    {
        Effect* emptyEffect = TestEffects::CreateTestEffect(m_sharedTestState.getScene());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*emptyEffect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const ramses::internal::DataLayout geometryLayout = m_sharedTestState.getInternalScene().getDataLayout(geometry->impl().getAttributeDataLayout());
        const ramses::internal::ResourceContentHash& effectHashFromGeometryLayout = geometryLayout.getEffectHash();

        EXPECT_EQ(emptyEffect->impl().getLowlevelResourceHash(), effectHashFromGeometryLayout);

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getScene().destroy(*emptyEffect);
    }

    TEST_P(GeometryTest, indicesFieldIsCreatedAtFixedSlot)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const ramses::internal::DataFieldHandle indicesField(GeometryImpl::IndicesDataFieldIndex);
        const ramses::internal::EFixedSemantics semantics = m_sharedTestState.getInternalScene().getDataLayout(geometry->impl().getAttributeDataLayout()).getField(indicesField).semantics;
        EXPECT_EQ(ramses::internal::EFixedSemantics::Indices, semantics);

        m_sharedTestState.getScene().destroy(*geometry);
    }

    TEST_P(GeometryTest, canSetResource)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const vec3f vert{ 0.f, 1.f, 2.f };
        ArrayResource* const vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInput.has_value());
        EXPECT_TRUE(geometry->setInputBuffer(*optInput, *vertices, 13u));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(3u), *vertices, 13u); // first field is indices

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getScene().destroy(*vertices);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingResourceWithMismatchingTypeInEffect)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const vec2f vert{ 1.f, 2.f };
        ArrayResource* const vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInput.has_value());
        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getScene().destroy(*vertices);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingVec2ArrayResourceFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const vec2f vert{ 1.f, 2.f };
        ramses::Scene& anotherScene(*m_sharedTestState.getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const vertices = anotherScene.createArrayResource(1u, &vert, "vec2Vertices");
        ASSERT_TRUE(vertices != nullptr);

        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());
        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getClient().destroy(anotherScene);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingIndexDataBufferFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ramses::Scene* otherScene = m_sharedTestState.getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const indices = otherScene->createArrayBuffer(ramses::EDataType::UInt32, 3u, "indices");
        ASSERT_NE(nullptr, indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        m_sharedTestState.getScene().destroy(*indices);
        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getClient().destroy(*otherScene);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingVertexDataBufferFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ramses::Scene* otherScene = m_sharedTestState.getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const vertices = otherScene->createArrayBuffer(ramses::EDataType::Float, 3u, "vertices");
        ASSERT_NE(nullptr, vertices);

        const auto optInput = m_sharedTestState.effect->findAttributeInput("floatArrayInput");
        ASSERT_TRUE(optInput.has_value());
        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        m_sharedTestState.getScene().destroy(*vertices);
        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getClient().destroy(*otherScene);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingArrayBufferByteBlobFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ramses::Scene* otherScene = m_sharedTestState.getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        ArrayBuffer* const vertices = otherScene->createArrayBuffer(ramses::EDataType::ByteBlob, 3u, "vertices");
        ASSERT_NE(nullptr, vertices);

        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        constexpr uint16_t nonZeroStride = 13u;
        EXPECT_FALSE(geometry->setInputBuffer(*optInputVec2, *vertices, 0u, nonZeroStride));
        EXPECT_FALSE(geometry->setInputBuffer(*optInputVec3, *vertices, 2 * sizeof(float), nonZeroStride));

        m_sharedTestState.getScene().destroy(*vertices);
        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getClient().destroy(*otherScene);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingArrayResourceByteBlobFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ramses::Scene* otherScene = m_sharedTestState.getClient().createScene(sceneId_t(777u));
        ASSERT_NE(nullptr, otherScene);

        const std::byte data[4] = { std::byte{0} };
        ArrayResource* const vertices = otherScene->createArrayResource(sizeof(data), data);
        ASSERT_NE(nullptr, vertices);


        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        constexpr uint16_t nonZeroStride = 13u;
        EXPECT_FALSE(geometry->setInputBuffer(*optInputVec2, *vertices, 0u, nonZeroStride));
        EXPECT_FALSE(geometry->setInputBuffer(*optInputVec3, *vertices, 2 * sizeof(float), nonZeroStride));

        m_sharedTestState.getScene().destroy(*vertices);
        m_sharedTestState.getScene().destroy(*geometry);
        m_sharedTestState.getClient().destroy(*otherScene);
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingUint16ArrayIndicesFromAnotherScene)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        uint16_t inds[3] = { 0u };
        ramses::Scene& anotherScene(*m_sharedTestState.getClient().createScene(sceneId_t{ 0xf00 }));
        ArrayResource* const indices = anotherScene.createArrayResource(3u, inds, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl().getIndicesCount());
        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        m_sharedTestState.getClient().destroy(anotherScene);
    }

    TEST_P(GeometryTest, canSetIndicesResource16)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint16_t inds[3] = { 0u };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(3u, inds, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl().getIndicesCount());
        EXPECT_TRUE(geometry->setIndices(*indices));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl().getElementCount(), geometry->impl().getIndicesCount());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, canSetIndicesResource32)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const uint32_t inds[3] = { 0u };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(3u, inds, "indices");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl().getIndicesCount());
        EXPECT_TRUE(geometry->setIndices(*indices));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(0u), *indices, 0u);
        EXPECT_EQ(indices->impl().getElementCount(), geometry->impl().getIndicesCount());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, canSetIndicesDataBuffer16)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::UInt16, 1u, "index data buffer");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl().getIndicesCount());
        EXPECT_TRUE(geometry->setIndices(*indices));
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(0u), indices->impl(), 0u);
        EXPECT_EQ(indices->impl().getElementCount(), geometry->impl().getIndicesCount());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, canSetIndicesDataBuffer32)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::UInt32, 1u, "index data buffer");
        ASSERT_TRUE(indices != nullptr);

        EXPECT_EQ(0u, geometry->impl().getIndicesCount());
        EXPECT_TRUE(geometry->setIndices(*indices));
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(0u), indices->impl(), 0u);
        EXPECT_EQ(indices->impl().getElementCount(), geometry->impl().getIndicesCount());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceFLoat)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(4u, inds, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec2F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const vec2f indice{ 1.f, 2.f };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(1u, &indice, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec3F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const vec3f indice{ 1.f, 2.f, 3.f };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(1u, &indice, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceVec4F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const vec4f indice{ 1.f, 2.f, 3.f, 4.f };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(1u, &indice, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferFloat)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Float, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec2F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Vector2F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec3F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Vector3F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferVec4F)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float inds[4] = { .0f, .0f, .0f, .0f };
        ArrayBuffer* indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Vector4F, 1u, "indices");
        ASSERT_TRUE(indices);
        indices->updateData(0u, 1u, inds);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayBufferByteBlob)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const indices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::ByteBlob, 1u, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSetIndicesWithWrongTypeArrayResourceByteBlob)
    {
        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const std::byte inds[4] = { std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3} };
        ArrayResource* const indices = m_sharedTestState.getScene().createArrayResource(sizeof(inds), inds, "indices");
        ASSERT_TRUE(indices);

        EXPECT_FALSE(geometry->setIndices(*indices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indices));
    }

    TEST_P(GeometryTest, canSetAttributeResourceInput)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const vec3f vert{ 0.f, 1.f, 2.f };
        ArrayResource* const vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_TRUE(geometry->setInputBuffer(*optInput, *vertices));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(3u), *vertices, 0u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, canSetAttributeVertexDataBufferInput)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const vertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Vector3F, 3u, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_TRUE(geometry->setInputBuffer(*optInput, *vertices, 16u));
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(3u), vertices->impl(), 16u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, canVertexDataBufferInput_ArrayBufferByteBlob)
    {
        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const interleavedVertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::ByteBlob, 5 * sizeof(float) *3u, "vertices");
        ASSERT_TRUE(interleavedVertices != nullptr);

        constexpr uint16_t nonZeroStride = 17u;
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(2u), interleavedVertices->impl(), 0u);
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(3u), interleavedVertices->impl(), 0u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*interleavedVertices));
    }

    TEST_P(GeometryTest, canSetVertexDataBufferInput_ArrayResourceByteBlob)
    {
        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const float data[10] = { 1.f };
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) interleaved vertices passed as byte blob
        ArrayResource* const interleavedVertices = m_sharedTestState.getScene().createArrayResource(sizeof(data), reinterpret_cast<const std::byte*>(data));
        ASSERT_TRUE(interleavedVertices != nullptr);

        constexpr uint16_t nonZeroStride = 17u;
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(2u), *interleavedVertices, 0u);
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(3u), *interleavedVertices, 0u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*interleavedVertices));
    }

    TEST_P(GeometryTest, canSetArrayBufferByteBlobToSingleAttribute)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::ByteBlob, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_TRUE(geometry->setInputBuffer(*optInput, *vertices));
        CheckDataBufferSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(2u), vertices->impl(), 0u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, canSetArrayResourceByteBlobToSingleAttribute)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const float data[10] = { 1.f };
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) interleaved vertices passed as byte blob
        ArrayResource* const vertices = m_sharedTestState.getScene().createArrayResource(sizeof(data), reinterpret_cast<const std::byte*>(data));
        ASSERT_TRUE(vertices);

        EXPECT_TRUE(geometry->setInputBuffer(*optInput, *vertices));
        CheckHashSetToInternalScene(*geometry, ramses::internal::DataFieldHandle(2u), *vertices, 0u);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, cannotSetStrideAndOffsetToNonByteBlobVertexDataBuffers)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Vector2F, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices, 1u, 2u));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, cannotSetStrideAndOffsetToNonByteBlobVertexArrayResource)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        const vec2f vert{ 0.f, 1.f };
        const auto vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices, 1u, 2u));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingAttributeResourceInputWithWrongType)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        const vec3f vert{ 0.f, 1.f, 2.f };
        ArrayResource* const vertices = m_sharedTestState.getScene().createArrayResource(1u, &vert, "vertices");
        ASSERT_TRUE(vertices != nullptr);

        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingAttributeVertexDataBufferInputWithWrongTypeUInt16)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::UInt16, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, reportsErrorWhenSettingAttributeVertexDataBufferInputWithWrongTypeUInt32)
    {
        const auto optInput = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInput.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry);

        ArrayBuffer* const vertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::UInt32, 1u, "vertices");
        ASSERT_TRUE(vertices);

        EXPECT_FALSE(geometry->setInputBuffer(*optInput, *vertices));

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertices));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedEffectResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*m_sharedTestState.effect));
        m_sharedTestState.effect = nullptr;
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        // restore the effect in sharedTestState after this test case
        m_sharedTestState.effect = TestEffectCreator::CreateEffect(m_sharedTestState.getScene(), false, GetParam());
        ASSERT_TRUE(nullptr != m_sharedTestState.effect);

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedIndicesResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedInputFloatArrayResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedInputVec2ArrayResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedInputVec3ArrayResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedInputVec4ArrayResource)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedArrayBufferByteBlob)
    {
        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        ArrayBuffer* const interleavedVertices = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::ByteBlob, 5 * sizeof(float) * 3u, "vertices");
        ASSERT_TRUE(interleavedVertices != nullptr);
        std::vector<std::byte> dummyData(interleavedVertices->getMaximumNumberOfElements(), std::byte{0x00});
        interleavedVertices->updateData(0u, 1u, dummyData.data());

        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        constexpr uint16_t nonZeroStride = 12u;
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*interleavedVertices));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedArrayResourceByteBlobl)
    {
        const auto optInputVec2 = m_sharedTestState.effect->findAttributeInput("vec2fArrayInput");
        ASSERT_TRUE(optInputVec2.has_value());
        const auto optInputVec3 = m_sharedTestState.effect->findAttributeInput("vec3fArrayInput");
        ASSERT_TRUE(optInputVec3.has_value());

        Geometry* const geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");
        ASSERT_TRUE(geometry != nullptr);

        uint32_t data[4] = { 0u };
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) interleaved vertices passed as byte blob
        ArrayResource* const interleavedVertices = m_sharedTestState.getScene().createArrayResource(sizeof(data), reinterpret_cast<const std::byte*>(data));
        ASSERT_TRUE(interleavedVertices != nullptr);

        auto floatArray = SetFloatArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        constexpr uint16_t nonZeroStride = 12u;
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec2, *interleavedVertices, 0u, nonZeroStride));
        EXPECT_TRUE(geometry->setInputBuffer(*optInputVec3, *interleavedVertices, 2 * sizeof(float), nonZeroStride));
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*interleavedVertices));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*floatArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithDestroyedVertexDataBuffer)
    {
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");

        ArrayBuffer* const vertexDataBuffer = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Float, 3, "vertices");
        ASSERT_TRUE(vertexDataBuffer != nullptr);
        const auto optInput = m_sharedTestState.effect->findAttributeInput("floatArrayInput");
        ASSERT_TRUE(optInput.has_value());
        geometry->setInputBuffer(*optInput, *vertexDataBuffer);
        const float data[] = { 0 };
        vertexDataBuffer->updateData(0u, 1u, data);

        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertexDataBuffer));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }

    TEST_P(GeometryTest, reportsErrorWhenValidatedWithVertexDataBufferThatHasWrongType)
    {
        //It is possible that a data buffer gets deleted, and new data buffer gets created with same
        //handle. Unfortunately validate can not check if a data buffer was destroyed and re-created
        //but it can at least check that the assigned data buffer is of a correct type
        Geometry* geometry = m_sharedTestState.getScene().createGeometry(*m_sharedTestState.effect, "geometry");

        ArrayBuffer* const vertexDataBuffer = m_sharedTestState.getScene().createArrayBuffer(ramses::EDataType::Float, 3, "vertices");
        ASSERT_TRUE(vertexDataBuffer != nullptr);
        const auto optInput = m_sharedTestState.effect->findAttributeInput("floatArrayInput");
        ASSERT_TRUE(optInput.has_value());
        geometry->setInputBuffer(*optInput, *vertexDataBuffer);
        const float data[] = { 0 };
        vertexDataBuffer->updateData(0u, 1u, data);

        auto vec2fArray = SetVec2fArrayInput(*geometry);
        auto vec3fArray = SetVec3fArrayInput(*geometry);
        auto vec4fArray = SetVec4fArrayInput(*geometry);
        auto indicesArray = SetIndicesInput(*geometry);
        ValidationReport report;
        geometry->validate(report);
        EXPECT_FALSE(report.hasIssue());

        //delete data buffer and create new one with same handle
        ramses::internal::DataBufferHandle dataBufferHandle = vertexDataBuffer->impl().getDataBufferHandle();
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vertexDataBuffer));
        ASSERT_FALSE(m_sharedTestState.getScene().impl().getIScene().isDataBufferAllocated(dataBufferHandle));
        m_sharedTestState.getScene().impl().getIScene().allocateDataBuffer(ramses::internal::EDataBufferType::VertexBuffer, ramses::internal::EDataType::Vector2F, 10 * sizeof(float), dataBufferHandle);
        ASSERT_TRUE(m_sharedTestState.getScene().impl().getIScene().isDataBufferAllocated(dataBufferHandle));
        report.clear();
        geometry->validate(report);
        EXPECT_TRUE(report.hasError());

        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*geometry));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec2fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec3fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*vec4fArray));
        EXPECT_TRUE(m_sharedTestState.getScene().destroy(*indicesArray));
    }
}
