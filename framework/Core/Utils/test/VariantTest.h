//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VARIANTTEST_H
#define RAMSES_VARIANTTEST_H

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/Variant.h"

namespace ramses_internal
{
    template <typename T>
    class VariantTest : public testing::Test
    {
    public:
        VariantTest();

    protected:
        T m_value;
    };
}

#endif
