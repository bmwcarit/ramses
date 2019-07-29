//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include <gtest/gtest.h>
#include "RamsesClientImpl.h"
#include "Utils/File.h"
#include "ramses-client-api/RamsesClient.h"
#include "gtest/gtest-typed-test.h"
#include "RamsesObjectTestTypes.h"
#include "RamsesObjectTestTypes.h"
#include "ClientTestUtils.h"
#include "ramses-client-api/AnimationSystemObjectIterator.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimatedSetter.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineStepFloat.h"
#include "ramses-client-api/SplineStepVector4i.h"
#include "ramses-client-api/SplineStepVector4f.h"
#include "ramses-client-api/SplineStepVector3i.h"
#include "ramses-client-api/SplineStepVector3f.h"
#include "ramses-client-api/SplineStepVector2i.h"
#include "ramses-client-api/SplineStepVector2f.h"
#include "ramses-client-api/SplineStepInt32.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector4i.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineLinearVector3i.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector2i.h"
#include "ramses-client-api/SplineLinearVector2f.h"
#include "ramses-client-api/SplineLinearInt32.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/SplineBezierVector4i.h"
#include "ramses-client-api/SplineBezierVector4f.h"
#include "ramses-client-api/SplineBezierVector3i.h"
#include "ramses-client-api/SplineBezierVector3f.h"
#include "ramses-client-api/SplineBezierVector2i.h"
#include "ramses-client-api/SplineBezierVector2f.h"
#include "ramses-client-api/SplineBezierInt32.h"
#include "ramses-utils.h"
#include "RamsesObjectTypeTraits.h"

namespace ramses
{
    template <typename SceneObjectType>
    class AnimationSystemObjectIteratorTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        AnimationSystemObjectIteratorTest() : LocalTestClientWithSceneAndAnimationSystem()
        {
        }
        ramses::ERamsesObjectType getTypeIDForObjectType()
        {
            return ramses::TYPE_ID_OF_RAMSES_OBJECT<SceneObjectType>::ID;
        }
    };

    TYPED_TEST_CASE(AnimationSystemObjectIteratorTest, ramses::AnimationObjectTypes);

    TYPED_TEST(AnimationSystemObjectIteratorTest, ObjectIteratorReturnsMultipleObjects)
    {
        const RamsesObject* ro1 = &this->template createObject<TypeParam>("ro1");
        const RamsesObject* ro2 = &this->template createObject<TypeParam>("ro2");

        AnimationSystemObjectIterator iterator(this->animationSystem, this->getTypeIDForObjectType());

        RamsesObject* iteratedObject1 = iterator.getNext();
        RamsesObject* iteratedObject2 = iterator.getNext();

        ASSERT_TRUE(nullptr != iteratedObject1);
        ASSERT_TRUE(nullptr != iteratedObject2);
        EXPECT_TRUE(ro1 == iteratedObject1);
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(*ro1));
        EXPECT_TRUE(ro2 == iteratedObject2);
        EXPECT_TRUE(nullptr != RamsesUtils::TryConvert<TypeParam>(*ro2));
        EXPECT_TRUE(nullptr == iterator.getNext());
    }

    TYPED_TEST(AnimationSystemObjectIteratorTest, ObjectIteratorDirectlyReturnsNullIfNoObjectsAvailable)
    {
        AnimationSystemObjectIterator iterator(this->animationSystem, this->getTypeIDForObjectType());

        EXPECT_TRUE(nullptr == iterator.getNext());
    }
}
