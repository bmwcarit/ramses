//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SceneTest.h"
#include "Animation/AnimationSystem.h"

using namespace testing;

namespace ramses_internal
{
    TYPED_TEST_SUITE(AScene, SceneTypes);

    TYPED_TEST(AScene, ContainsCreatedAnimationSystem_ClientVersion)
    {
        SceneActionCollection dummyCollection;
        AnimationSystemFactory animSystemFactory(EAnimationSystemOwner_Client, &dummyCollection);
        IAnimationSystem* animSystem = animSystemFactory.createAnimationSystem(EAnimationSystemFlags_FullProcessing, AnimationSystemSizeInformation());

        auto id = this->m_scene.addAnimationSystem(animSystem);

        EXPECT_TRUE(this->m_scene.getAnimationSystem(id) != nullptr);
        EXPECT_EQ(static_cast<UInt32>(EAnimationSystemFlags_FullProcessing), animSystem->getFlags());
    }

    TYPED_TEST(AScene, ContainsCreatedAnimationSystem_RendererVersion)
    {
        AnimationSystemFactory animSystemFactory(EAnimationSystemOwner_Renderer);
        IAnimationSystem* animSystem = animSystemFactory.createAnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation());

        auto id = this->m_scene.addAnimationSystem(animSystem);

        EXPECT_TRUE(this->m_scene.getAnimationSystem(id) != nullptr);
        EXPECT_NE(0u, animSystem->getFlags() & EAnimationSystemFlags_FullProcessing);
    }

    TYPED_TEST(AScene, ContainsCreatedAnimationSystem_SceneManagerVersion)
    {
        AnimationSystemFactory animSystemFactory(EAnimationSystemOwner_Scenemanager);
        IAnimationSystem* animSystem = animSystemFactory.createAnimationSystem(EAnimationSystemFlags_FullProcessing, AnimationSystemSizeInformation());

        auto id = this->m_scene.addAnimationSystem(animSystem);

        EXPECT_TRUE(this->m_scene.getAnimationSystem(id) != nullptr);
        EXPECT_EQ(0u, animSystem->getFlags() & EAnimationSystemFlags_FullProcessing);
    }

    TYPED_TEST(AScene, RetrieveListOfAnimationSystems)
    {
        IAnimationSystem* animSystem1 = new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation());
        IAnimationSystem* animSystem2 = new AnimationSystem(EAnimationSystemFlags_FullProcessing, AnimationSystemSizeInformation());

        auto id1 = this->m_scene.addAnimationSystem(animSystem1);
        auto id2 = this->m_scene.addAnimationSystem(animSystem2);

        bool id1Found = false;
        bool id2Found = false;
        for (auto handle = AnimationSystemHandle(0); handle < this->m_scene.getAnimationSystemCount(); ++handle)
        {
            if (this->m_scene.isAnimationSystemAllocated(handle))
            {
                if (id1 == handle)
                {
                    EXPECT_FALSE(id1Found);
                    id1Found = true;
                }
                if (id2 == handle)
                {
                    EXPECT_FALSE(id2Found);
                    id2Found = true;
                }
            }
        }
        EXPECT_TRUE(id1Found);
        EXPECT_TRUE(id2Found);
    }

    TYPED_TEST(AScene, RemoveAnimationSystem)
    {
        IAnimationSystem* animSystem1 = new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation());
        IAnimationSystem* animSystem2 = new AnimationSystem(EAnimationSystemFlags_FullProcessing, AnimationSystemSizeInformation());

        auto id1 = this->m_scene.addAnimationSystem(animSystem1);
        auto id2 = this->m_scene.addAnimationSystem(animSystem2);

        EXPECT_TRUE(this->m_scene.getAnimationSystem(id2) != nullptr);

        this->m_scene.removeAnimationSystem(id2);

        EXPECT_EQ(nullptr, this->m_scene.getAnimationSystem(id2));

        bool id1Found = false;
        bool id2Found = false;
        for (auto handle = AnimationSystemHandle(0); handle < this->m_scene.getAnimationSystemCount(); ++handle)
        {
            if (this->m_scene.isAnimationSystemAllocated(handle))
            {
                if (id1 == handle)
                {
                    EXPECT_FALSE(id1Found);
                    id1Found = true;
                }
                if (id2 == handle)
                {
                    EXPECT_FALSE(id2Found);
                    id2Found = true;
                }
            }
        }
        EXPECT_TRUE(id1Found);
        EXPECT_FALSE(id2Found);
    }
}
