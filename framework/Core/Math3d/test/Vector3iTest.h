//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTOR3ITEST_H
#define RAMSES_VECTOR3ITEST_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Math3d/Vector3i.h"

class Vector3iTest: public testing::Test
{
public:
    void SetUp();
    void TearDown();
protected:
    ramses_internal::Vector3i vec1;
private:
};

#endif
