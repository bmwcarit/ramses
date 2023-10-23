//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "gmock/gmock.h"


namespace ramses::internal
{
    class ContextMock : public IContext
    {
    public:
        ContextMock();
        ~ContextMock() override;

        MOCK_METHOD(bool, init, ()); // Does not exist in IContext, needed for only for testing

        MOCK_METHOD(bool,  swapBuffers, (), (override));
        MOCK_METHOD(bool,  enable, (), (override));
        MOCK_METHOD(bool,  disable, (), (override));

        MOCK_METHOD(DeviceResourceMapper&, getResources, (), (override));
        MOCK_METHOD(void*, getProcAddress, (const char*), (const, override));
    };
}
