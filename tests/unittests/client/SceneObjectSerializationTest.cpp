//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cstdint>
#include <string_view>
#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/Scene/SceneMergeHandleMapping.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "ramses/framework/EScenePublicationMode.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/Node.h"
#include "ramses/client/ramses-utils.h"
#include "impl/SerializationContext.h"
#include "impl/SceneImpl.h"
#include "impl/SceneObjectImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/SerializationHelper.h"

using namespace testing;

namespace ramses::internal
{
    class ASceneObjectSerializationTest : public LocalTestClient, public testing::Test
    {
    public:
        const std::string_view name{"TestNode"};
        const NodeHandle nodeHandle{0};
        const sceneObjectId_t sceneObjectId{1};
        const std::pair<uint64_t, uint64_t> userId{987u, 654u};

        void serializeTest(BinaryOutputStream& outStream)
        {
            SceneConfig config{sceneId_t(123u), EScenePublicationMode::LocalOnly};
            auto * scene = getClient().createScene(config);
            ASSERT_NE(nullptr, scene);

            auto* node = scene->createNode(name);
            ASSERT_NE(nullptr, node);

            node->setUserId(userId.first, userId.second);

            EXPECT_EQ(nodeHandle, node->impl().getNodeHandle());
            EXPECT_EQ(name, node->impl().getName());
            EXPECT_EQ(sceneObjectId, node->impl().getSceneObjectId());
            EXPECT_EQ(userId, node->impl().getUserId());

            SaveFileConfigImpl saveFileConfig;
            SerializationContext serializationContext(saveFileConfig);
            ASSERT_TRUE(node->impl().serialize(outStream, serializationContext));

            getClient().destroy(*scene);
        }
    };

    TEST_F(ASceneObjectSerializationTest, canSerializeAndDeserializeNode)
    {
        BinaryOutputStream outStream;
        serializeTest(outStream);

        SceneConfig config{sceneId_t(123u), EScenePublicationMode::LocalOnly};
        auto * scene = getClient().createScene(config);
        ASSERT_NE(nullptr, scene);

        auto* node = scene->createNode(name);
        ASSERT_NE(nullptr, node);

        BinaryInputStream inStream(outStream.getData());
        SceneConfigImpl sceneConfig;
        DeserializationContext deserializationContext(sceneConfig, nullptr);
        deserializationContext.resize(1, 1);

        ObjectIDType objectID = SerializationHelper::DeserializeObjectID(inStream);
        EXPECT_NE(DeserializationContext::GetObjectIDNull(), objectID);
        ASSERT_TRUE(node->impl().deserialize(inStream, deserializationContext));

        EXPECT_EQ(nodeHandle, node->impl().getNodeHandle());
        EXPECT_EQ(name, node->impl().getName());
        EXPECT_EQ(sceneObjectId, node->impl().getSceneObjectId());
        EXPECT_EQ(userId, node->impl().getUserId());

        getClient().destroy(*scene);
    }

    TEST_F(ASceneObjectSerializationTest, canSerializeAndDeserializeNodeWithMapping)
    {
        BinaryOutputStream outStream;
        serializeTest(outStream);

        SceneConfig config{sceneId_t(123u), EScenePublicationMode::LocalOnly};
        auto * scene = getClient().createScene(config);
        ASSERT_NE(nullptr, scene);

        auto* node = scene->createNode();
        ASSERT_NE(nullptr, node);

        auto* mappedNode = scene->createNode(name);
        ASSERT_NE(nullptr, mappedNode);

        BinaryInputStream inStream(outStream.getData());
        SceneConfigImpl sceneConfig;
        SceneMergeHandleMapping mapping;
        const NodeHandle mappedNodeHandle{1};
        const sceneObjectId_t mappedSceneObjectId{2};
        mapping.addMapping(nodeHandle, mappedNodeHandle);

        DeserializationContext deserializationContext(sceneConfig, &mapping);
        deserializationContext.resize(2, 2);

        ObjectIDType objectID = SerializationHelper::DeserializeObjectID(inStream);
        EXPECT_NE(DeserializationContext::GetObjectIDNull(), objectID);
        ASSERT_TRUE(mappedNode->impl().deserialize(inStream, deserializationContext));

        EXPECT_EQ(mappedNodeHandle, mappedNode->impl().getNodeHandle());
        EXPECT_EQ(name, mappedNode->impl().getName());
        EXPECT_EQ(mappedSceneObjectId, mappedNode->impl().getSceneObjectId());
        EXPECT_EQ(userId, mappedNode->impl().getUserId());

        EXPECT_TRUE(mapping.hasMapping(sceneObjectId));
        EXPECT_EQ(mappedSceneObjectId, mapping.getMapping(sceneObjectId));

        getClient().destroy(*scene);
    }
}
