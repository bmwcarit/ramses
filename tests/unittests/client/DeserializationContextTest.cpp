//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "impl/SerializationContext.h"
#include "impl/SceneConfigImpl.h"
#include "internal/SceneGraph/Scene/SceneMergeHandleMapping.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"

using namespace testing;

namespace ramses::internal
{
    template <typename T>
    class DeserializationContextTest : public ::testing::Test
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
        SceneReferenceHandle>;

    TYPED_TEST_SUITE(DeserializationContextTest, HandleTypes);

    TYPED_TEST(DeserializationContextTest, returnsMappedHandle)
    {
        SceneConfigImpl sceneConfigImpl;

        SceneMergeHandleMapping mapping;
        TypeParam handle{42u};
        TypeParam handle2{13u};
        TypeParam mappedHandle{99u};
        mapping.addMapping(handle, mappedHandle);
        DeserializationContext deserializationContext(sceneConfigImpl, &mapping);
        EXPECT_NE(nullptr, deserializationContext.getSceneMergeHandleMapping());

        // write two handles to stream
        BinaryOutputStream outStream;
        outStream << handle;
        outStream << handle2;

        BinaryInputStream inStream(outStream.getData());

        // deserialize mapped handle
        TypeParam deserializedHandle;
        deserializationContext.deserializeAndMap(inStream, deserializedHandle);
        EXPECT_EQ(mappedHandle, deserializedHandle);
    }
}
