//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SCENEACTIONUTILSTEST_H
#define RAMSES_SCENEACTIONUTILSTEST_H

#include "gtest/gtest.h"
#include "Scene/SceneActionUtils.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class SceneActionVectorUtilsTest : public testing::Test
    {
    public:
        SceneActionVectorUtilsTest()
        : actionsWithOneTestAction(createSceneActionVector(
            {
                ESceneActionId_AllocateNode,
                ESceneActionId_AllocateCamera,
                ESceneActionId_TestAction,
                ESceneActionId_AllocateBlitPass
            }
            ))

        , actionsWithThreeTestAction(createSceneActionVector(
            {
                ESceneActionId_AllocateNode,
                ESceneActionId_TestAction,
                ESceneActionId_AllocateCamera,
                ESceneActionId_TestAction,
                ESceneActionId_TestAction,
                ESceneActionId_AllocateBlitPass
            }
            ))

        , actionsWithNoTestAction(createSceneActionVector(
            {
                ESceneActionId_AllocateNode,
                ESceneActionId_AllocateBlitPass,
                ESceneActionId_AllocateBlitPass
            }
            ))

        , actionsWithMultipleAction(createSceneActionVector(
            {
                ESceneActionId_SetStateStencilFunc,
                ESceneActionId_AllocateCamera,
                ESceneActionId_TestAction,
                ESceneActionId_AllocateCamera,
                ESceneActionId_ReleaseDataLayout,
                ESceneActionId_AllocateCamera,
                ESceneActionId_AllocateRenderable,
                ESceneActionId_AllocateCamera,
                ESceneActionId_TestAction,
                ESceneActionId_TestAction,
                ESceneActionId_AllocateBlitPass
            }
            ))
        {
            singleType.push_back(ESceneActionId_TestAction);

            multiType.push_back(ESceneActionId_TestAction);
            multiType.push_back(ESceneActionId_AllocateCamera);
        }

        static SceneActionCollection createSceneActionVector(const std::vector<ramses_internal::ESceneActionId>& types)
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
        EXPECT_EQ(1u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithOneTestAction, ESceneActionId_TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithMultipleOccurence)
    {
        EXPECT_EQ(3u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithThreeTestAction, ESceneActionId_TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithNoOccurence)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsWithNoTestAction, ESceneActionId_TestAction));
    }

    TEST_F(SceneActionVectorUtilsTest, CountSingleTypeWithEmptyVector)
    {
        EXPECT_EQ(0u, SceneActionCollectionUtils::CountNumberOfActionsOfType(actionsEmpty, ESceneActionId_TestAction));
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

#endif
