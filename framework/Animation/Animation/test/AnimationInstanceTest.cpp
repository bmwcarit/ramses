//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformMath.h"
#include "AnimationInstanceTest.h"
#include "Animation/AnimationInstance.h"
#include "gtest/gtest.h"

using namespace testing;

namespace ramses_internal
{
    TEST_F(AnimationInstanceTest, InitializedToInvalid)
    {
        const AnimationInstance animInst;
        EXPECT_EQ(AnimationInstance::InvalidInstance().getSplineHandle(), animInst.getSplineHandle());
        EXPECT_EQ(AnimationInstance::InvalidInstance().getInterpolationType(), animInst.getInterpolationType());
    }

    TEST_F(AnimationInstanceTest, AddDataBinding)
    {
        AnimationInstance animInst;
        const DataBindHandle handle1(99u);
        const DataBindHandle handle2(111u);

        EXPECT_EQ(0u, animInst.getDataBindings().size());

        animInst.addDataBinding(handle1);
        EXPECT_EQ(1u, animInst.getDataBindings().size());

        animInst.addDataBinding(handle2);
        EXPECT_EQ(2u, animInst.getDataBindings().size());

        EXPECT_TRUE(animInst.hasDataBinding(handle1));
        EXPECT_TRUE(animInst.hasDataBinding(handle2));
    }

    TEST_F(AnimationInstanceTest, AddExistingDataBinding)
    {
        AnimationInstance animInst;
        const DataBindHandle handle1(99u);
        const DataBindHandle handle2(111u);

        EXPECT_EQ(0u, animInst.getDataBindings().size());

        animInst.addDataBinding(handle1);
        EXPECT_EQ(1u, animInst.getDataBindings().size());

        animInst.addDataBinding(handle2);
        EXPECT_EQ(2u, animInst.getDataBindings().size());

        animInst.addDataBinding(handle1);
        animInst.addDataBinding(handle2);
        EXPECT_EQ(2u, animInst.getDataBindings().size());

        EXPECT_TRUE(animInst.hasDataBinding(handle1));
        EXPECT_TRUE(animInst.hasDataBinding(handle2));
    }
}
