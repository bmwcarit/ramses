//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/Spline.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector4i.h"

#include "RamsesClientImpl.h"
#include "SceneImpl.h"
#include "AnimationSystemImpl.h"
#include "ClientTestUtils.h"
#include "RamsesObjectTestTypes.h"

namespace ramses
{
    using namespace testing;

    template <typename ObjectType>
    class AnimationSystemOwnershipTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    protected:
        void expectNoFrameworkObjectsAllocated()
        {
            m_creationHelper.destroyAdditionalAllocatedAnimationSystemObjects();
            const ramses_internal::IAnimationSystem& animSystem = this->animationSystem.impl.getIAnimationSystem();
            for (ramses_internal::SplineHandle i(0); i < animSystem.getTotalSplineCount(); ++i)
            {
                EXPECT_FALSE(animSystem.containsSpline(i));
            }
            for (ramses_internal::DataBindHandle i(0); i < animSystem.getTotalDataBindCount(); ++i)
            {
                EXPECT_FALSE(animSystem.containsDataBinding(i));
            }
            for (ramses_internal::AnimationInstanceHandle i(0); i < animSystem.getTotalAnimationInstanceCount(); ++i)
            {
                EXPECT_FALSE(animSystem.containsAnimationInstance(i));
            }
            for (ramses_internal::AnimationHandle i(0); i < animSystem.getTotalAnimationCount(); ++i)
            {
                EXPECT_FALSE(animSystem.containsAnimation(i));
            }
        }
    };

    TYPED_TEST_SUITE(AnimationSystemOwnershipTest, AnimationObjectTypes);

    TYPED_TEST(AnimationSystemOwnershipTest, animationSystemObjectsAreOfTypeAnimationObject)
    {
        const AnimationObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType_AnimationObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType_SceneObject));
        EXPECT_TRUE(obj->isOfType(ERamsesObjectType_ClientObject));
    }

    TYPED_TEST(AnimationSystemOwnershipTest, animationSystemObjectsHaveReferenceToTheirAnimationSystem)
    {
        const AnimationObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(&this->animationSystem.impl, &obj->impl.getAnimationSystemImpl());
        EXPECT_EQ(&this->animationSystem.impl.getIAnimationSystem(), &obj->impl.getIAnimationSystem());
        EXPECT_EQ(&this->m_scene.impl, &obj->impl.getSceneImpl());
        EXPECT_EQ(&this->m_scene.impl.getIScene(), &obj->impl.getIScene());
        EXPECT_EQ(&this->client.impl, &obj->impl.getClientImpl());
    }

    TYPED_TEST(AnimationSystemOwnershipTest, animationSystemContainsCreatedObject)
    {
        const RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        const RamsesObject* sn = this->animationSystem.findObjectByName("objectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(AnimationSystemOwnershipTest, animationSystemDoesNotContainDestroyedObject)
    {
        AnimationObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        EXPECT_EQ(StatusOK, this->animationSystem.destroy(*obj));
        const RamsesObject* sn = this->animationSystem.findObjectByName("objectName");
        ASSERT_TRUE(nullptr == sn);
        this->expectNoFrameworkObjectsAllocated();
    }

    TYPED_TEST(AnimationSystemOwnershipTest, animationObjectNameChanged)
    {
        RamsesObject* obj = &this->template createObject<TypeParam>("objectName");
        ASSERT_TRUE(nullptr != obj);
        obj->setName("otherObjectName");
        RamsesObject* sn = this->animationSystem.findObjectByName("otherObjectName");
        ASSERT_TRUE(nullptr != sn);
        EXPECT_EQ(obj, sn);
    }

    TYPED_TEST(AnimationSystemOwnershipTest, animationSystemNotContainsDestroyedObject)
    {
        RamsesObject& obj = this->template  createObject<TypeParam>("objectName");
        TypeParam& objTyped = static_cast<TypeParam&>(obj);

        this->animationSystem.destroy(objTyped);
        const RamsesObject* sn = this->animationSystem.findObjectByName("objectName");
        EXPECT_EQ(nullptr, sn);
    }
}
