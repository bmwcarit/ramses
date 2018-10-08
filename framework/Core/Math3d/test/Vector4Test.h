//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR4TEST_H
#define RAMSES_VECTOR4TEST_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include <Math3d/Vector4.h>

class Vector4Test: public testing::Test
{
public:
    void SetUp();
    void TearDown();
protected:
    ramses_internal::Vector4 vec1;
private:
};

#endif
