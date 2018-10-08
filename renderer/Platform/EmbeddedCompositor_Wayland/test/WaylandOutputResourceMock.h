//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDOUTPUTRESOURCEMOCK_H
#define RAMSES_WAYLANDOUTPUTRESOURCEMOCK_H

#include "gmock/gmock.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputResource.h"
#include "Collections/String.h"

namespace ramses_internal
{
    class WaylandOutputResourceMock : public WaylandOutputResource
    {
    public:
        MOCK_METHOD0(getVersion, int());
        MOCK_METHOD2(postError, void(uint32_t code, const String& message));
        MOCK_METHOD0(getUserData, void*());
        MOCK_METHOD3(setImplementation, void(const void* implementation, void* data, IWaylandResourceDestroyFuncT destroy));
        MOCK_METHOD1(addDestroyListener, void(WaylandListener* listener));
        MOCK_METHOD0(getWaylandNativeResource, void*());
        MOCK_METHOD8(outputSendGeometry, void(int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform));
        MOCK_METHOD4(outputSendMode, void(uint32_t flags, int32_t width, int32_t height, int32_t refresh));
        MOCK_METHOD1(outputSendScale, void(int32_t factor));
        MOCK_METHOD0(outputSendDone, void());
        MOCK_METHOD0(disownWaylandResource, void());
    };
}

#endif
