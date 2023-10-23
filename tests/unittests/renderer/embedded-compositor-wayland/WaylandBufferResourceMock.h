//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBufferResource.h"

namespace ramses::internal
{
    class WaylandBufferResourceMock : public WaylandBufferResource
    {
    public:
        MOCK_METHOD(int, getVersion, (), (override));
        MOCK_METHOD(void, postError, (uint32_t code, const std::string& message), (override));
        MOCK_METHOD(void*, getUserData, (), (override));
        MOCK_METHOD(void, setImplementation, (const void* implementation, void* data, IWaylandResourceDestroyFuncT destroyCallback), (override));
        MOCK_METHOD(void, addDestroyListener, (wl_listener* listener), (override));
        MOCK_METHOD(wl_resource*, getLowLevelHandle, (), (override));
        MOCK_METHOD(int32_t, getWidth, (), (const, override));
        MOCK_METHOD(int32_t, getHeight, (), (const, override));
        MOCK_METHOD(const void*, bufferGetSharedMemoryData, (), (const, override));
        MOCK_METHOD(void, bufferSendRelease, (), (override));
        MOCK_METHOD(void, destroy, (), (override));
        MOCK_METHOD(WaylandBufferResource*, clone, (), (const, override));
    };
}
