//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "internal/SceneGraph/Scene/SceneMergeHandleMapping.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

using namespace testing;

namespace ramses::internal
{
    template <typename T>
    class SceneMergeHandleMappingTest : public ::testing::Test
    {
    };

    using HandleTypes = ::testing::Types<
        RenderableHandle,
        RenderStateHandle,
        CameraHandle,
        NodeHandle,
        TransformHandle,
        DataLayoutHandle,
        DataInstanceHandle,
        UniformBufferHandle,
        TextureSamplerHandle,
        RenderGroupHandle,
        RenderPassHandle,
        BlitPassHandle,
        PickableObjectHandle,
        RenderTargetHandle,
        RenderBufferHandle,
        DataBufferHandle,
        TextureBufferHandle,
        DataSlotHandle,
        SceneReferenceHandle,
        sceneObjectId_t>;

    TYPED_TEST_SUITE(SceneMergeHandleMappingTest, HandleTypes);

    TYPED_TEST(SceneMergeHandleMappingTest, returnsMappedHandle)
    {
        SceneMergeHandleMapping mapping;
        TypeParam handle(42u);
        TypeParam mappedHandle(49u);

        mapping.addMapping(handle, mappedHandle);
        auto returnedHandle = mapping.getMapping(handle);

        EXPECT_EQ(returnedHandle, mappedHandle);
    }

    TYPED_TEST(SceneMergeHandleMappingTest, returnsHasMapping)
    {
        SceneMergeHandleMapping mapping;
        TypeParam handle(42u);
        TypeParam mappedHandle(49u);

        EXPECT_FALSE(mapping.hasMapping(handle));
        EXPECT_FALSE(mapping.hasMapping(TypeParam::Invalid()));

        mapping.addMapping(handle, mappedHandle);
        EXPECT_TRUE(mapping.hasMapping(handle));
    }

    TYPED_TEST(SceneMergeHandleMappingTest, returnsInvalidHandleWhenNotMapped)
    {
        SceneMergeHandleMapping mapping;
        TypeParam handle(42u);

        auto returnedHandle = mapping.getMapping(handle);
        EXPECT_FALSE(returnedHandle.isValid());
    }
}
