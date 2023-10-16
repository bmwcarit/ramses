//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "impl/RamsesClientImpl.h"
#include "internal/Core/Utils/File.h"
#include "ramses/client/SceneIterator.h"
#include "ClientTestUtils.h"

namespace ramses::internal
{
    class ASceneIterator : public LocalTestClientWithScene, public ::testing::Test
    {
    };

    TEST_F(ASceneIterator, returnsCorrectSceneWhenDereferenced)
    {
        ramses::SceneIterator iter(client);

        ramses::Scene* sceneFromIterator = iter.getNext();
        ASSERT_TRUE(nullptr != sceneFromIterator);
        // id of only existing scene - the default test scene
        EXPECT_EQ(sceneId_t(123u), sceneFromIterator->getSceneId());
        EXPECT_TRUE(nullptr == iter.getNext());
    }

    TEST_F(ASceneIterator, returnsCorrectSceneWhenDereferenced_WithSeveralScenes)
    {
        client.createScene(sceneId_t(1u));
        ramses::SceneIterator iter(client);

        ramses::Scene* sceneFromIterator = iter.getNext();
        ramses::Scene* scene2FromIterator = iter.getNext();
        ASSERT_TRUE(nullptr != sceneFromIterator);
        ASSERT_TRUE(nullptr != scene2FromIterator);
        EXPECT_TRUE(sceneFromIterator != scene2FromIterator);
        EXPECT_TRUE(nullptr == iter.getNext());
    }

    TEST_F(ASceneIterator, doesNotReturnDestroyedScene)
    {
        ramses::Scene* scene2FromIterator = client.createScene(sceneId_t(1u));
        client.destroy(*scene2FromIterator);
        ramses::SceneIterator iter(client);

        // one scene always created in class ctor
        EXPECT_TRUE(&m_scene == iter.getNext());
        EXPECT_TRUE(nullptr == iter.getNext());
    }

    class ASceneIteratorWithoutAvailableScenes : public LocalTestClient, public ::testing::Test
    {
    };

    TEST_F(ASceneIteratorWithoutAvailableScenes, returnsNullOnGetNext)
    {
        ramses::SceneIterator iter(client);

        EXPECT_TRUE(nullptr == iter.getNext());
    }

}
