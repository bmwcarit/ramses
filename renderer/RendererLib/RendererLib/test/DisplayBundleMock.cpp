//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayBundleMock.h"
#include <atomic>

using namespace testing;

namespace ramses_internal
{
    DisplayBundleMock::DisplayBundleMock()
    {
        static std::atomic_int dummy;
        ON_CALL(*this, traceId()).WillByDefault(ReturnRef(dummy));
        EXPECT_CALL(*this, traceId()).Times(AnyNumber());
    }

    DisplayBundleMock::~DisplayBundleMock() = default;
}
