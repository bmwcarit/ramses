//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererSceneResourceRegistry.h"
#include "SceneAPI/EDataBufferType.h"

using namespace testing;
using namespace ramses_internal;

class ARendererSceneResourceRegistry : public ::testing::Test
{
public:
protected:
    RendererSceneResourceRegistry registry;
};

TEST_F(ARendererSceneResourceRegistry, doesNotContainAnythingInitially)
{
    RenderBufferHandleVector rbs;
    RenderTargetHandleVector rts;
    BlitPassHandleVector bps;
    StreamTextureHandleVector sts;
    DataBufferHandleVector dbs;
    TextureBufferHandleVector tbs;

    registry.getAllRenderBuffers(rbs);
    registry.getAllRenderTargets(rts);
    registry.getAllBlitPasses(bps);
    registry.getAllStreamTextures(sts);
    registry.getAllDataBuffers(dbs);
    registry.getAllTextureBuffers(tbs);


    EXPECT_TRUE(rbs.empty());
    EXPECT_TRUE(rts.empty());
    EXPECT_TRUE(bps.empty());
    EXPECT_TRUE(sts.empty());
    EXPECT_TRUE(dbs.empty());
    EXPECT_TRUE(tbs.empty());
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveRenderBuffer)
{
    const RenderBufferHandle rb(13u);
    const DeviceResourceHandle deviceHandle(123u);
    registry.addRenderBuffer(rb, deviceHandle, 0u, false);

    EXPECT_EQ(deviceHandle, registry.getRenderBufferDeviceHandle(rb));

    RenderBufferHandleVector rbs;
    registry.getAllRenderBuffers(rbs);
    ASSERT_EQ(1u, rbs.size());
    EXPECT_EQ(rb, rbs[0]);
    rbs.clear();

    registry.removeRenderBuffer(rb);
    registry.getAllRenderBuffers(rbs);
    EXPECT_TRUE(rbs.empty());
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveRenderTarget)
{
    const RenderTargetHandle rt(13u);
    const DeviceResourceHandle deviceHandle(123u);
    registry.addRenderTarget(rt, deviceHandle);

    EXPECT_EQ(deviceHandle, registry.getRenderTargetDeviceHandle(rt));

    RenderTargetHandleVector rts;
    registry.getAllRenderTargets(rts);
    ASSERT_EQ(1u, rts.size());
    EXPECT_EQ(rt, rts[0]);
    rts.clear();

    registry.removeRenderTarget(rt);
    registry.getAllRenderTargets(rts);
    EXPECT_TRUE(rts.empty());
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveBlitPass)
{
    const BlitPassHandle bp(13u);
    const DeviceResourceHandle deviceHandle1(123u);
    const DeviceResourceHandle deviceHandle2(124u);
    registry.addBlitPass(bp, deviceHandle1, deviceHandle2);

    DeviceResourceHandle deviceHandle1actual;
    DeviceResourceHandle deviceHandle2actual;
    registry.getBlitPassDeviceHandles(bp, deviceHandle1actual, deviceHandle2actual);
    EXPECT_EQ(deviceHandle1, deviceHandle1actual);
    EXPECT_EQ(deviceHandle2, deviceHandle2actual);

    BlitPassHandleVector bps;
    registry.getAllBlitPasses(bps);
    ASSERT_EQ(1u, bps.size());
    EXPECT_EQ(bp, bps[0]);
    bps.clear();

    registry.removeBlitPass(bp);
    registry.getAllBlitPasses(bps);
    EXPECT_TRUE(bps.empty());
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveStreamTexture)
{
    const StreamTextureHandle st(13u);
    const WaylandIviSurfaceId source(666u);
    registry.addStreamTexture(st, source);

    StreamTextureHandleVector sts;
    registry.getAllStreamTextures(sts);
    ASSERT_EQ(1u, sts.size());
    EXPECT_EQ(st, sts[0]);
    sts.clear();

    registry.removeStreamTexture(st);
    registry.getAllStreamTextures(sts);
    EXPECT_TRUE(sts.empty());
}

TEST_F(ARendererSceneResourceRegistry, canGetStreamTextureData)
{
    const StreamTextureHandle st(13u);
    const WaylandIviSurfaceId source(666u);
    registry.addStreamTexture(st, source);
    EXPECT_EQ(source, registry.getStreamTextureSourceId(st));
    registry.removeStreamTexture(st);
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveDataBuffers)
{
    const DataBufferHandle db(13u);
    const DeviceResourceHandle deviceHandle(123u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    registry.addDataBuffer(db, deviceHandle, dataBufferType, 0u);

    DataBufferHandleVector dbs;
    registry.getAllDataBuffers(dbs);
    ASSERT_EQ(1u, dbs.size());
    EXPECT_EQ(db, dbs[0]);
    dbs.clear();

    registry.removeDataBuffer(db);
    registry.getAllDataBuffers(dbs);
    EXPECT_TRUE(dbs.empty());
}

TEST_F(ARendererSceneResourceRegistry, canGetDataBufferDeviceHandle)
{
    const DataBufferHandle db(13u);
    const DeviceResourceHandle deviceHandle(123u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    registry.addDataBuffer(db, deviceHandle, dataBufferType, 0u);

    EXPECT_EQ(deviceHandle, registry.getDataBufferDeviceHandle(db));
    registry.removeDataBuffer(db);
}

TEST_F(ARendererSceneResourceRegistry, canGetDataBufferType)
{
    const DataBufferHandle db(13u);
    const DeviceResourceHandle deviceHandle(123u);
    const EDataBufferType dataBufferType = EDataBufferType::IndexBuffer;
    registry.addDataBuffer(db, deviceHandle, dataBufferType, 0u);

    EXPECT_EQ(dataBufferType, registry.getDataBufferType(db));
    registry.removeDataBuffer(db);
}

TEST_F(ARendererSceneResourceRegistry, canAddAndRemoveTextureBuffers)
{
    const TextureBufferHandle tb(13u);
    const DeviceResourceHandle deviceHandle(123u);
    registry.addTextureBuffer(tb, deviceHandle, ETextureFormat::RG8, 0u);

    TextureBufferHandleVector tbs;
    registry.getAllTextureBuffers(tbs);
    ASSERT_EQ(1u, tbs.size());
    EXPECT_EQ(tb, tbs[0]);
    tbs.clear();

    registry.removeTextureBuffer(tb);
    registry.getAllTextureBuffers(tbs);
    EXPECT_TRUE(tbs.empty());
}

TEST_F(ARendererSceneResourceRegistry, canGetTextureBufferAttributes)
{
    const TextureBufferHandle tb(13u);
    const DeviceResourceHandle deviceHandle(123u);
    registry.addTextureBuffer(tb, deviceHandle, ETextureFormat::RG8, 0u);

    EXPECT_EQ(deviceHandle, registry.getTextureBufferDeviceHandle(tb));
    EXPECT_EQ(ETextureFormat::RG8, registry.getTextureBufferFormat(tb));
    registry.removeTextureBuffer(tb);
}
