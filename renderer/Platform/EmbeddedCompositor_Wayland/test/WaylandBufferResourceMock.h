//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDBUFFERRESOURCEMOCK_H
#define RAMSES_WAYLANDBUFFERRESOURCEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"

namespace ramses_internal
{
    class WaylandBufferResourceMock : public WaylandBufferResource
    {
    public:
        MOCK_METHOD(int, getVersion, (), (override));
        MOCK_METHOD(void, postError, (uint32_t code, const String& message), (override));
        MOCK_METHOD(void*, getUserData, (), (override));
        MOCK_METHOD(void, setImplementation, (const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy), (override));
        MOCK_METHOD(void, addDestroyListener, (wl_listener* listener), (override));
        MOCK_METHOD(void*, getWaylandNativeResource, (), (override));
        MOCK_METHOD(int32_t, bufferGetSharedMemoryWidth, (), (const, override));
        MOCK_METHOD(int32_t, bufferGetSharedMemoryHeight, (), (const, override));
        MOCK_METHOD(const void*, bufferGetSharedMemoryData, (), (const, override));
        MOCK_METHOD(void, bufferSendRelease, (), (override));
        MOCK_METHOD(void, disownWaylandResource, (), (override));
        MOCK_METHOD(WaylandBufferResource*, clone, (), (const, override));
    };
}

#endif
