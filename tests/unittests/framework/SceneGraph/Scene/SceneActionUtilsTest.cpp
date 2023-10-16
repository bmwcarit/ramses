//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneActionUtils.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class SceneActionVectorUtilsTest : public testing::Test
    {
    public:
        SceneActionVectorUtilsTest()
        : actionsWithOneTestAction(createSceneActionVector(
            {
                ESceneActionId::AllocateNode,
                ESceneActionId::AllocateCamera,
                ESceneActionId::TestAction,
                ESceneActionId::AllocateBlitPass
            }
            ))

        , actionsWithThreeTestAction(createSceneActionVector(
            {
                ESceneActionId::AllocateNode,
                ESceneActionId::TestAction,
                ESceneActionId::AllocateCamera,
                ESceneActionId::TestAction,
                ESceneActionId::TestAction,
                ESceneActionId::AllocateBlitPass
            }
            ))

        , actionsWithNoTestAction(createSceneActionVector(
            {
                ESceneActionId::AllocateNode,
                ESceneActionId::AllocateBlitPass,
                ESceneActionId::AllocateBlitPass
            }
            ))

        , actionsWithMultipleAction(createSceneActionVector(
            {
                ESceneActionId::SetStateStencilFunc,
                ESceneActionId::AllocateCamera,
                ESceneActionId::TestAction,
                ESceneActionId::AllocateCamera,
                ESceneActionId::ReleaseDataLayout,
                ESceneActionId::AllocateCamera,
                ESceneActionId::AllocateRenderable,
                ESceneActionId::AllocateCamera,
                ESceneActionId::TestAction,
                ESceneActionId::TestAction,
                ESceneActionId::AllocateBlitPass
            }
            ))
        {
            singleType.push_back(ESceneActionId::TestAction);

            multiType.push_back(ESceneActionId::TestAction);
            multiType.push_back(ESceneActionId::AllocateCamera);
        }

        static SceneActionCollection createSceneActionVector(const std::vector<ramses::internal::ESceneActionId>& types)
        {
            SceneActionCollection result;
            for(auto type : types)
            {
                result.addRawSceneActionInformation(type, 0);  // fake, empty sceneactions
            }
            return result;
        }

        SceneActionCollection actionsWithOneTestAction;
        SceneActionCollection actionsWithThreeTestAction;
        SceneActionCollection actionsWithNoTestAction;
        SceneActionCollection actionsWithMultipleAction;
        SceneActionCollection actionsEmpty;

        SceneActionIdVector singleType;
        SceneActionIdVector multiType;
    };

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithSingleOccurence)
    {
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithOneTestAction, ESceneActionId::TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithMultipleOccurence)
    {
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithThreeTestAction, ESceneActionId::TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithNoOccurence)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithNoTestAction, ESceneActionId::TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithEmptyVector)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsEmpty, ESceneActionId::TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountVectorTypeWithSingleOccurence)
    {
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithOneTestAction, singleType));
        EXPECT_EQ(2u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithOneTestAction, multiType));
    }

    TEST_F(SceneActionVectorUtilsTest, CountVectorTypeWithMultipleOccurence)
    {
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithMultipleAction, singleType));
        EXPECT_EQ(7u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithMultipleAction, multiType));
    }

    TEST_F(SceneActionVectorUtilsTest, CountVectorTypeWithNoOccurence)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithNoTestAction, singleType));
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithNoTestAction, multiType));
    }

    TEST_F(SceneActionVectorUtilsTest, CountVectorTypeWithEmptyVector)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsEmpty, singleType));
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsEmpty, multiType));
    }
}
