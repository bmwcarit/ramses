//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONTIMETEST_H
#define RAMSES_ANIMATIONTIMETEST_H

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Animation/AnimationTime.h"

namespace ramses_internal
{
    class AnimationTimeTest : public testing::Test
    {
    public:
        AnimationTimeTest()  {}

    protected:
        AnimationTime m_time;
    };
}

#endif
