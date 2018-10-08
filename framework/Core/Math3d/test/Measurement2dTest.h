//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEASUREMENT2DTEST_H
#define RAMSES_MEASUREMENT2DTEST_H

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Math3d/Measurement2d.h"

namespace ramses_internal
{
    class Measurement2dTest : public testing::Test
    {
    public:
        Measurement2dTest();
        ~Measurement2dTest();
        void SetUp();
        void TearDown();
    protected:
        Measurement2d<Float> mMeasurement;
    };
}

#endif
