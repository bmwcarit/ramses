//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SURFACEMOCK_H
#define RAMSES_SURFACEMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/ISurface.h"

#include "ContextMock.h"
#include "WindowMock.h"

namespace ramses_internal
{
    template <template<typename> class MOCK_TYPE>
    class SurfaceMock : public ISurface
    {
    public:
        SurfaceMock();
        ~SurfaceMock() override;

        MOCK_METHOD(Bool, enable, (), (override));
        MOCK_METHOD(Bool, disable, (), (override));
        MOCK_METHOD(ramses_internal::Bool, swapBuffers, (), (override));
        MOCK_METHOD(void, frameRendered, (), (override));
        MOCK_METHOD(Bool, canRenderNewFrame, (), (const, override));

        MOCK_METHOD(IWindow&, getWindow, (), (override));
        MOCK_METHOD(const IWindow&, getWindow, (), (const, override));
        MOCK_METHOD(IContext&, getContext, (), (override));
        MOCK_METHOD(const IContext&, getContext, (), (const, override));

        MOCK_TYPE<ContextMock> contextMock;
        MOCK_TYPE<WindowMock>  windowMock;
    };

    class SurfaceMockWithDestructor : public SurfaceMock< ::testing::StrictMock>
    {
    public:
        SurfaceMockWithDestructor();
        ~SurfaceMockWithDestructor();

        MOCK_METHOD(void, Die, ());
    };

    typedef SurfaceMock< ::testing::NiceMock> SurfaceNiceMock;
    typedef SurfaceMock< ::testing::StrictMock> SurfaceStrictMock;
}

#endif
