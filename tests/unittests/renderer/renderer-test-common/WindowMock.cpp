//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WindowMock.h"

using namespace testing;

namespace ramses::internal
{
    WindowMock::WindowMock()
    {
        createDefaultMockCalls();
    }

    void WindowMock::createDefaultMockCalls()
    {
        ON_CALL(*this, getHeight()).WillByDefault(Return(FakeHeight));
        ON_CALL(*this, getWidth()).WillByDefault(Return(FakeWidth));
        ON_CALL(*this, getAspectRatio()).WillByDefault(Return(static_cast<float>(FakeWidth) / FakeHeight));
        ON_CALL(*this, getWaylandIviSurfaceID()).WillByDefault(Return(WaylandIviSurfaceId()));

        EXPECT_CALL(*this, getWidth()).Times(AnyNumber());
        EXPECT_CALL(*this, getHeight()).Times(AnyNumber());
    }

    WindowMock::~WindowMock() = default;
}

