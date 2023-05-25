//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NATIVEWAYLANDRESOURCEMOCK_H
#define RAMSES_NATIVEWAYLANDRESOURCEMOCK_H

#include "EmbeddedCompositor_Wayland/INativeWaylandResource.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class NativeWaylandResourceMock: public INativeWaylandResource
    {
    public:
        MOCK_METHOD(int, getVersion, (), (override));
        MOCK_METHOD(void, postError, (uint32_t code, const String& message), (override));
        MOCK_METHOD(void*, getUserData, (), (override));
        MOCK_METHOD(void, setImplementation, (const void* implementation, void* data, IWaylandResourceDestroyFuncT destroyCallback), (override));
        MOCK_METHOD(void, addDestroyListener, (wl_listener* listener), (override));
        MOCK_METHOD(wl_resource*, getLowLevelHandle, (), (override));
        MOCK_METHOD(void, destroy, (), (override));
    };

    class NativeWaylandResourceMockWithDestructor: public NativeWaylandResourceMock
    {
    public:
        MOCK_METHOD(void, Die, (), ());
        ~NativeWaylandResourceMockWithDestructor() override
        {
            Die();
        }
    };
}

#endif
