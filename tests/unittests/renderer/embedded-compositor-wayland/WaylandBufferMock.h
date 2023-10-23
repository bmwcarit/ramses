//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandBuffer.h"

namespace ramses::internal
{
    class WaylandBufferMock : public IWaylandBuffer
    {
    public:
        MOCK_METHOD(WaylandBufferResource&, getResource, (), (const, override));
        MOCK_METHOD(void, reference, (), (override));
        MOCK_METHOD(void, release, (), (override));
        MOCK_METHOD(bool, isSharedMemoryBuffer, (), (const, override));
        MOCK_METHOD(void, logInfos, (RendererLogContext&, const WaylandEGLExtensionProcs&), (const, override));
    };
}
