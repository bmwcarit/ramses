//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_MATRIX44TEST_H
#define RAMSES_MATRIX44TEST_H

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class Matrix44Test: public testing::Test
    {
    public:
        Matrix44Test();
    protected:
        ramses_internal::Matrix44f mat1;
    private:
    };

}
#endif
