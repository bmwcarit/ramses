//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "internal/RendererLib/RendererSceneResourceRegistry.h"
#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"

using namespace testing;

namespace ramses::internal
{
    class ARendererSceneResourceRegistry : public ::testing::Test
    {
    public:
    protected:
        RendererSceneResourceRegistry registry;
    };

    TEST_F(ARendererSceneResourceRegistry, doesNotContainAnythingInitially)
    {
        EXPECT_TRUE(registry.getAll<RenderBufferHandle>().empty());
        EXPECT_TRUE(registry.getAll<RenderTargetHandle>().empty());
        EXPECT_TRUE(registry.getAll<BlitPassHandle>().empty());
        EXPECT_TRUE(registry.getAll<DataBufferHandle>().empty());
        EXPECT_TRUE(registry.getAll<TextureBufferHandle>().empty());
        EXPECT_TRUE(registry.getAll<UniformBufferHandle>().empty());
        EXPECT_TRUE(registry.getAll<SemanticUniformBufferHandle>().empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveRenderBuffer)
    {
        const RenderBufferHandle rb(13u);
        const DeviceResourceHandle deviceHandle(123u);
        const RenderBuffer props{ 16u, 8u, EPixelStorageFormat::R16F, ERenderBufferAccessMode::ReadWrite, 2u };
        registry.add(rb, deviceHandle, 10u, props);

        EXPECT_EQ(deviceHandle, registry.get(rb).deviceHandle);

        EXPECT_EQ(10u, registry.get(rb).size);
        EXPECT_EQ(props.width, registry.get(rb).renderBufferProperties.width);
        EXPECT_EQ(props.height, registry.get(rb).renderBufferProperties.height);
        EXPECT_EQ(props.format, registry.get(rb).renderBufferProperties.format);
        EXPECT_EQ(props.accessMode, registry.get(rb).renderBufferProperties.accessMode);
        EXPECT_EQ(props.sampleCount, registry.get(rb).renderBufferProperties.sampleCount);

        auto rbs = registry.getAll<RenderBufferHandle>();
        ASSERT_EQ(1u, rbs.size());
        EXPECT_EQ(rb, rbs[0]);
        rbs.clear();

        registry.remove(rb);
        rbs = registry.getAll<RenderBufferHandle>();
        EXPECT_TRUE(rbs.empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveRenderTarget)
    {
        const RenderTargetHandle rt(13u);
        const DeviceResourceHandle deviceHandle(123u);
        registry.add(rt, deviceHandle);

        EXPECT_EQ(deviceHandle, registry.get(rt));

        auto rts = registry.getAll<RenderTargetHandle>();
        ASSERT_EQ(1u, rts.size());
        EXPECT_EQ(rt, rts[0]);
        rts.clear();

        registry.remove(rt);
        rts = registry.getAll<RenderTargetHandle>();
        EXPECT_TRUE(rts.empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveBlitPass)
    {
        const BlitPassHandle bp(13u);
        const DeviceResourceHandle deviceHandle1(123u);
        const DeviceResourceHandle deviceHandle2(124u);
        registry.add(bp, deviceHandle1, deviceHandle2);

        DeviceResourceHandle deviceHandle1actual;
        DeviceResourceHandle deviceHandle2actual;
        registry.getBlitPassDeviceHandles(bp, deviceHandle1actual, deviceHandle2actual);
        EXPECT_EQ(deviceHandle1, deviceHandle1actual);
        EXPECT_EQ(deviceHandle2, deviceHandle2actual);

        auto bps = registry.getAll<BlitPassHandle>();
        ASSERT_EQ(1u, bps.size());
        EXPECT_EQ(bp, bps[0]);
        bps.clear();

        registry.remove(bp);
        bps = registry.getAll<BlitPassHandle>();
        EXPECT_TRUE(bps.empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveDataBuffers)
    {
        const DataBufferHandle db(13u);
        const DeviceResourceHandle deviceHandle(123u);
        const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
        registry.add(db, deviceHandle, dataBufferType, 0u);

        auto dbs = registry.getAll<DataBufferHandle>();
        ASSERT_EQ(1u, dbs.size());
        EXPECT_EQ(db, dbs[0]);
        dbs.clear();

        registry.remove(db);
        dbs = registry.getAll<DataBufferHandle>();
        EXPECT_TRUE(dbs.empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canGetDataBufferDeviceHandle)
    {
        const DataBufferHandle db(13u);
        const DeviceResourceHandle deviceHandle(123u);
        const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
        registry.add(db, deviceHandle, dataBufferType, 0u);

        EXPECT_EQ(deviceHandle, registry.get(db).deviceHandle);
        registry.remove(db);
    }

    TEST_F(ARendererSceneResourceRegistry, canGetDataBufferType)
    {
        const DataBufferHandle db(13u);
        const DeviceResourceHandle deviceHandle(123u);
        const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
        registry.add(db, deviceHandle, dataBufferType, 0u);

        EXPECT_EQ(dataBufferType, registry.get(db).dataBufferType);
        registry.remove(db);
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveTextureBuffers)
    {
        const TextureBufferHandle tb(13u);
        const DeviceResourceHandle deviceHandle(123u);
        registry.add(tb, deviceHandle, EPixelStorageFormat::RG8, 0u);

        auto tbs = registry.getAll<TextureBufferHandle>();
        ASSERT_EQ(1u, tbs.size());
        EXPECT_EQ(tb, tbs[0]);
        tbs.clear();

        registry.remove(tb);
        tbs = registry.getAll<TextureBufferHandle>();
        EXPECT_TRUE(tbs.empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canGetTextureBufferAttributes)
    {
        const TextureBufferHandle tb(13u);
        const DeviceResourceHandle deviceHandle(123u);
        registry.add(tb, deviceHandle, EPixelStorageFormat::RG8, 0u);

        EXPECT_EQ(deviceHandle, registry.get(tb).deviceHandle);
        EXPECT_EQ(EPixelStorageFormat::RG8, registry.get(tb).format);
        registry.remove(tb);
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveUniformBuffers)
    {
        const UniformBufferHandle ub1{ 13u };
        const UniformBufferHandle ub2{ 14u };
        const DeviceResourceHandle deviceHandle1{ 123u };
        const DeviceResourceHandle deviceHandle2{ 124u };
        registry.add(ub1, deviceHandle1);
        registry.add(ub2, deviceHandle2);

        EXPECT_THAT(registry.getAll<UniformBufferHandle>(), ::testing::UnorderedElementsAre(ub1, ub2));
        EXPECT_EQ(deviceHandle1, registry.get(ub1));
        EXPECT_EQ(deviceHandle2, registry.get(ub2));

        registry.remove(ub1);
        EXPECT_THAT(registry.getAll<UniformBufferHandle>(), ::testing::ElementsAre(ub2));
        EXPECT_EQ(deviceHandle2, registry.get(ub2));

        registry.remove(ub2);
        EXPECT_TRUE(registry.getAll<UniformBufferHandle>().empty());
    }

    TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveSemanticUniformBuffers)
    {
        const SemanticUniformBufferHandle ub1{ RenderableHandle{ 13u } };
        const SemanticUniformBufferHandle ub2{ CameraHandle{ 14u } };
        const DeviceResourceHandle deviceHandle1{ 123u };
        const DeviceResourceHandle deviceHandle2{ 124u };
        registry.add(ub1, deviceHandle1);
        registry.add(ub2, deviceHandle2);

        EXPECT_THAT(registry.getAll<SemanticUniformBufferHandle>(), ::testing::UnorderedElementsAre(ub1, ub2));
        EXPECT_EQ(deviceHandle1, registry.get(ub1));
        EXPECT_EQ(deviceHandle2, registry.get(ub2));

        registry.remove(ub1);
        EXPECT_THAT(registry.getAll<SemanticUniformBufferHandle>(), ::testing::ElementsAre(ub2));
        EXPECT_EQ(deviceHandle2, registry.get(ub2));

        registry.remove(ub2);
        EXPECT_TRUE(registry.getAll<SemanticUniformBufferHandle>().empty());
    }
}
