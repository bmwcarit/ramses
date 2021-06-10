//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "RendererAPI/Types.h"
#include "RendererLib/SceneResourceUploader.h"
#include "RendererResourceManagerMock.h"
#include "Scene/Scene.h"
#include "SceneAllocateHelper.h"
#include "SceneUtils/ResourceUtils.h"

namespace ramses_internal {
using namespace testing;

class ASceneResourceUploader : public ::testing::Test
{
public:
    ASceneResourceUploader()
        : scene(SceneInfo(sceneID))
        , allocateHelper(scene)
    {
    }

protected:
    const SceneId                           sceneID{ 13u };
    Scene                                   scene;
    SceneAllocateHelper                     allocateHelper;
    StrictMock<RendererResourceManagerMock> resourceManager;
};


TEST_F(ASceneResourceUploader, UploadVertexArray)
{
    const auto node = allocateHelper.allocateNode();
    const auto renderable = allocateHelper.allocateRenderable(node);
    const DataFieldInfo indexField(EDataType::Indices, 1u, EFixedSemantics::Indices);
    const DataFieldInfo vert1Field(EDataType::Vector2Buffer, 1u);
    const DataFieldInfo vert2Field(EDataType::Vector3Buffer, 1u);

    const ResourceContentHash fakeEffectHash            {123456u, 0u};
    const ResourceContentHash fakeIndexBufferHash       {123456u, 1u};
    const ResourceContentHash fakeVertexBuffer1Hash     {123456u, 2u};
    const ResourceContentHash fakeVertexBuffer2tHash    {123456u, 3u};

    const DeviceResourceHandle fakeEffectDeviceHandle       { 999990u };
    const DeviceResourceHandle fakeIndexBufferDeviceHandle  { 999991u };
    const DeviceResourceHandle fakeVertexBuffer1DeviceHandle{ 999992u };
    const DeviceResourceHandle fakeVertexBuffer2DeviceHandle{ 999993u };

    const auto geomLayout = allocateHelper.allocateDataLayout({ indexField, vert1Field, vert2Field }, fakeEffectHash);
    const auto geomInstance = allocateHelper.allocateDataInstance(geomLayout);

    scene.setRenderableDataInstance(renderable, ERenderableDataSlotType_Geometry, geomInstance);
    scene.setRenderableStartVertex(renderable, 17u);
    scene.setDataResource(geomInstance, DataFieldHandle{ 0u }, fakeIndexBufferHash, {}, 0u, 0u, 0u);
    scene.setDataResource(geomInstance, DataFieldHandle{ 1u }, fakeVertexBuffer1Hash, {}, 11u, 12u, 13u);
    scene.setDataResource(geomInstance, DataFieldHandle{ 2u }, fakeVertexBuffer2tHash, {}, 14u, 15u, 16u);

    VertexArrayInfo expectedVertexArrayInfo;
    expectedVertexArrayInfo.shader = fakeEffectDeviceHandle;
    expectedVertexArrayInfo.indexBuffer = fakeIndexBufferDeviceHandle;
    expectedVertexArrayInfo.vertexBuffers.push_back({ fakeVertexBuffer1DeviceHandle, DataFieldHandle{1u}, 11u, 17u, EDataType::Vector2F, 12u, 13u });
    expectedVertexArrayInfo.vertexBuffers.push_back({ fakeVertexBuffer2DeviceHandle, DataFieldHandle{2u}, 14u, 17u, EDataType::Vector3F, 15u, 16u });

    InSequence seq;
    EXPECT_CALL(resourceManager, getResourceDeviceHandle(fakeEffectHash))           .WillOnce(Return(fakeEffectDeviceHandle));
    EXPECT_CALL(resourceManager, getResourceDeviceHandle(fakeIndexBufferHash))      .WillOnce(Return(fakeIndexBufferDeviceHandle));
    EXPECT_CALL(resourceManager, getResourceDeviceHandle(fakeVertexBuffer1Hash))    .WillOnce(Return(fakeVertexBuffer1DeviceHandle));
    EXPECT_CALL(resourceManager, getResourceDeviceHandle(fakeVertexBuffer2tHash))   .WillOnce(Return(fakeVertexBuffer2DeviceHandle));

    VertexArrayInfo resultVertexArrayInfo;
    EXPECT_CALL(resourceManager, uploadVertexArray(renderable, _, sceneID)).WillOnce(Invoke([&](auto, auto vai, auto) {
        resultVertexArrayInfo = std::move(vai);
        return DeviceResourceHandle{};
        }));
    SceneResourceUploader::UploadVertexArray(scene, renderable, resourceManager);

    EXPECT_EQ(fakeEffectDeviceHandle, resultVertexArrayInfo.shader);
    EXPECT_EQ(fakeIndexBufferDeviceHandle, resultVertexArrayInfo.indexBuffer);

    ASSERT_EQ(2u, resultVertexArrayInfo.vertexBuffers.size());

    EXPECT_EQ(fakeVertexBuffer1DeviceHandle     , resultVertexArrayInfo.vertexBuffers[0].deviceHandle);
    EXPECT_EQ(DataFieldHandle{ 0u }             , resultVertexArrayInfo.vertexBuffers[0].field);
    EXPECT_EQ(11u                               , resultVertexArrayInfo.vertexBuffers[0].instancingDivisor);
    EXPECT_EQ(17u                               , resultVertexArrayInfo.vertexBuffers[0].startVertex);
    EXPECT_EQ(EDataType::Vector2Buffer          , resultVertexArrayInfo.vertexBuffers[0].bufferDataType);
    EXPECT_EQ(12u                               , resultVertexArrayInfo.vertexBuffers[0].offsetWithinElement);
    EXPECT_EQ(13u                               , resultVertexArrayInfo.vertexBuffers[0].stride);

    EXPECT_EQ(fakeVertexBuffer2DeviceHandle     , resultVertexArrayInfo.vertexBuffers[1].deviceHandle);
    EXPECT_EQ(DataFieldHandle{ 1u }             , resultVertexArrayInfo.vertexBuffers[1].field);
    EXPECT_EQ(14u                               , resultVertexArrayInfo.vertexBuffers[1].instancingDivisor);
    EXPECT_EQ(17u                               , resultVertexArrayInfo.vertexBuffers[1].startVertex);
    EXPECT_EQ(EDataType::Vector3Buffer          , resultVertexArrayInfo.vertexBuffers[1].bufferDataType);
    EXPECT_EQ(15u                               , resultVertexArrayInfo.vertexBuffers[1].offsetWithinElement);
    EXPECT_EQ(16u                               , resultVertexArrayInfo.vertexBuffers[1].stride);
}
}
