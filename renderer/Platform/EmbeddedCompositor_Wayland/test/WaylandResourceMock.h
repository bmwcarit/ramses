//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDRESOURCEMOCK_H
#define RAMSES_WAYLANDRESOURCEMOCK_H

#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class WaylandResourceMock: public IWaylandResource
    {
    public:
        MOCK_METHOD(int, getVersion, (), (override));
        MOCK_METHOD(void, postError, (uint32_t code, const String& message), (override));
        MOCK_METHOD(void*, getUserData, (), (override));
        MOCK_METHOD(void, setImplementation, (const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy), (override));
        MOCK_METHOD(void, addDestroyListener, (wl_listener* listener), (override));
        MOCK_METHOD(void*, getWaylandNativeResource, (), (override));
        MOCK_METHOD(void, disownWaylandResource, (), (override));
    };
}

#endif
