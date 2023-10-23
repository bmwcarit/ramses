//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/SceneDependencyChecker.h"
#include "internal/SceneGraph/Scene/Scene.h"
#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class ASceneDependencyChecker : public ::testing::Test
    {
    public:
        SceneDependencyChecker dependencyChecker;

        [[nodiscard]] static bool CheckSceneOrder(SceneId  sceneFirst, SceneId  sceneSecond, const SceneIdVector&  sceneList)
        {
            bool foundFirst = false;
            for (auto i : sceneList)
            {
                if (i == sceneFirst)
                {
                    foundFirst = true;
                }
                if (i == sceneSecond)
                {
                    if (foundFirst)
                    {
                        return true;
                    }
                    return false;
                }
            }
            return false;
        }
    };

    TEST_F(ASceneDependencyChecker, isEmptyInitially)
    {
        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canAddDependencyToIndependentScenes)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //check that the scenes are not dependent in th first place
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveDependencyOfDependentScenes)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));

        dependencyChecker.removeDependency(scene1, scene2);

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, returnsCorrectDependencyListForOneDependency)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        //check scene dependency list
        const SceneIdVector& sceneList = dependencyChecker.getDependentScenesInOrder();
        EXPECT_EQ(2u, sceneList.size());
        EXPECT_TRUE(CheckSceneOrder(scene1, scene2, sceneList));
    }

    TEST_F(ASceneDependencyChecker, canNotAddReverseDependencyToDependentScenes)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        //add reverse dependency should not be possible
        EXPECT_FALSE(dependencyChecker.addDependency(scene2, scene1));

        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canAddMultipleDependenciesForOneScene)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);

        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene3));

        //check scene dependency list
        const SceneIdVector& sceneList = dependencyChecker.getDependentScenesInOrder();
        EXPECT_EQ(3u, sceneList.size());
        EXPECT_TRUE(CheckSceneOrder(scene1, scene2, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene1, scene3, sceneList));

        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canAddMultipleDependenciesForMultipleScenes)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);
        SceneId scene4(17u);

        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene3, scene4));

        //check scene dependency list
        const SceneIdVector& sceneList = dependencyChecker.getDependentScenesInOrder();
        EXPECT_EQ(4u, sceneList.size());
        EXPECT_TRUE(CheckSceneOrder(scene1, scene2, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene3, scene4, sceneList));

        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canAddMultipleCascadingDependenciesForMultipleScenes)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);
        SceneId scene4(17u);

        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene3));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene4));

        //check scene dependency list
        const SceneIdVector& sceneList = dependencyChecker.getDependentScenesInOrder();
        EXPECT_EQ(4u, sceneList.size());
        EXPECT_TRUE(CheckSceneOrder(scene1, scene2, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene1, scene3, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene1, scene4, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene2, scene3, sceneList));
        EXPECT_TRUE(CheckSceneOrder(scene2, scene4, sceneList));

        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveSingleDependentScene)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        //check dependencies
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_EQ(2u, dependencyChecker.getDependentScenesInOrder().size());

        //remove scene
        dependencyChecker.removeScene(scene2);

        //check dependency removal for both scenes
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_EQ(0u, dependencyChecker.getDependentScenesInOrder().size());
        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveSingleSceneWithDependency)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        //check dependencies
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_EQ(2u, dependencyChecker.getDependentScenesInOrder().size());

        //remove scene
        dependencyChecker.removeScene(scene1);

        //check dependency removal for both scenes
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_EQ(0u, dependencyChecker.getDependentScenesInOrder().size());
        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveSingleDependentSceneFromCascadingDependency)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);
        SceneId scene4(17u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene3));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene4));

        //check dependencies
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene4));
        EXPECT_EQ(4u, dependencyChecker.getDependentScenesInOrder().size());

        //remove scene
        dependencyChecker.removeScene(scene1);

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene4));

        EXPECT_EQ(3u, dependencyChecker.getDependentScenesInOrder().size());
        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveSingleSceneWithDependencyFromCascadingDependency)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);
        SceneId scene4(17u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene3));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene4));

        //check dependencies
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene4));
        EXPECT_EQ(4u, dependencyChecker.getDependentScenesInOrder().size());

        //remove scene
        dependencyChecker.removeScene(scene4);

        //check dependency removal for both scenes
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene4));

        EXPECT_EQ(3u, dependencyChecker.getDependentScenesInOrder().size());
        EXPECT_FALSE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveSingleDependentSceneWithDependenciesFromCascadingDependency)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);
        SceneId scene3(7u);
        SceneId scene4(17u);

        //add a dependency
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene3));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene4));

        //check dependencies
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene4));
        EXPECT_EQ(4u, dependencyChecker.getDependentScenesInOrder().size());

        //remove scene
        dependencyChecker.removeScene(scene2);

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene4));

        EXPECT_EQ(0u, dependencyChecker.getDependentScenesInOrder().size());
        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, canRemoveDependencyAddedTwiceWithTwoRemovalsOnly)
    {
        SceneId scene1(3u);
        SceneId scene2(5u);

        //add a dependency twice
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene2));

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));

        dependencyChecker.removeDependency(scene1, scene2);

        //scene2 should still be dependent
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));

        dependencyChecker.removeDependency(scene1, scene2);

        //now it should not be dependent anymore
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));

        EXPECT_TRUE(dependencyChecker.isEmpty());
    }

    TEST_F(ASceneDependencyChecker, confidenceTest_complexDependencyHierarchy)
    {
        const SceneId scene1(1u);
        const SceneId scene2(2u);
        const SceneId scene3(3u);
        const SceneId scene4(4u);
        const SceneId scene5(5u);
        const SceneId scene6(6u);
        const SceneId scene7(7u);

        EXPECT_TRUE(dependencyChecker.addDependency(scene1, scene5));
        EXPECT_TRUE(dependencyChecker.addDependency(scene2, scene5));
        EXPECT_TRUE(dependencyChecker.addDependency(scene3, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene3, scene5));
        EXPECT_TRUE(dependencyChecker.addDependency(scene3, scene7));
        EXPECT_TRUE(dependencyChecker.addDependency(scene5, scene4));
        EXPECT_TRUE(dependencyChecker.addDependency(scene6, scene1));
        EXPECT_TRUE(dependencyChecker.addDependency(scene6, scene2));
        EXPECT_TRUE(dependencyChecker.addDependency(scene7, scene2));

        // fail to create cyclic dependency
        EXPECT_FALSE(dependencyChecker.addDependency(scene4, scene7));

        // query dependencies
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene4));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene5));
        EXPECT_TRUE(dependencyChecker.hasDependencyAsConsumer(scene7));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene6));

        // get ordered list of scenes
        const auto& orderedScenes = dependencyChecker.getDependentScenesInOrder();
        ASSERT_EQ(7u, orderedScenes.size());
        EXPECT_EQ(scene6, orderedScenes[0]);
        EXPECT_EQ(scene3, orderedScenes[1]);
        EXPECT_EQ(scene1, orderedScenes[2]);
        EXPECT_EQ(scene7, orderedScenes[3]);
        EXPECT_EQ(scene2, orderedScenes[4]);
        EXPECT_EQ(scene5, orderedScenes[5]);
        EXPECT_EQ(scene4, orderedScenes[6]);

        // remove dependencies
        dependencyChecker.removeDependency(scene1, scene5);
        dependencyChecker.removeDependency(scene2, scene5);
        dependencyChecker.removeDependency(scene3, scene2);
        dependencyChecker.removeDependency(scene3, scene5);
        dependencyChecker.removeDependency(scene3, scene7);
        dependencyChecker.removeDependency(scene5, scene4);
        dependencyChecker.removeDependency(scene6, scene1);
        dependencyChecker.removeDependency(scene6, scene2);
        dependencyChecker.removeDependency(scene7, scene2);

        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene1));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene2));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene3));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene4));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene5));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene6));
        EXPECT_FALSE(dependencyChecker.hasDependencyAsConsumer(scene7));

        EXPECT_TRUE(dependencyChecker.getDependentScenesInOrder().empty());
    }
}
