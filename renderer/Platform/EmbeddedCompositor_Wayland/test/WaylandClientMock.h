//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDCLIENTMOCK_H
#define RAMSES_WAYLANDCLIENTMOCK_H

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class WaylandClientMock : public IWaylandClient
    {
    public:
        WaylandClientMock()
        {
            EXPECT_CALL(*this, getCredentials()).Times(::testing::AnyNumber());
        }
        MOCK_METHOD(WaylandClientCredentials, getCredentials, (), (const, override));
        MOCK_METHOD(void, postNoMemory, (), (override));
        MOCK_METHOD(IWaylandResource*, resourceCreate, (const wl_interface* interface, int version, uint32_t id), (override));
        MOCK_METHOD(WaylandCallbackResource*, callbackResourceCreate, (const wl_interface* interface, int version, uint32_t id), (override));
    };
}

#endif
