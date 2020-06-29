//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDCLIENTMOCK_H
#define RAMSES_WAYLANDCLIENTMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

namespace ramses_internal
{
    class WaylandClientMock : public IWaylandClient
    {
    public:
        MOCK_METHOD(void, getCredentials, (pid_t& processId, uid_t& userId, gid_t& groupId), (override));
        MOCK_METHOD(void, postNoMemory, (), (override));
        MOCK_METHOD(IWaylandResource*, resourceCreate, (const wl_interface* interface, int version, uint32_t id), (override));
        MOCK_METHOD(WaylandCallbackResource*, callbackResourceCreate, (const wl_interface* interface, int version, uint32_t id), (override));
    };
}

#endif
