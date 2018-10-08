//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDRESOURCEMOCK_H
#define RAMSES_WAYLANDRESOURCEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"

namespace ramses_internal
{
    class WaylandResourceMock: public IWaylandResource
    {
    public:
        MOCK_METHOD0(getVersion, int());
        MOCK_METHOD2(postError, void(uint32_t code, const String& message));
        MOCK_METHOD0(getUserData, void*());
        MOCK_METHOD3(setImplementation, void(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy));
        MOCK_METHOD1(addDestroyListener, void(WaylandListener* listener));
        MOCK_METHOD0(getWaylandNativeResource, void*());
        MOCK_METHOD0(disownWaylandResource, void());
    };
}

#endif
