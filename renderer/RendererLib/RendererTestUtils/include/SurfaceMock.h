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

        MOCK_METHOD0(enable, Bool());
        MOCK_METHOD0(disable, Bool());
        MOCK_METHOD0(swapBuffers, ramses_internal::Bool());
        MOCK_METHOD0(frameRendered, void());
        MOCK_CONST_METHOD0(canRenderNewFrame, Bool());

        MOCK_METHOD0(getWindow, IWindow&());
        MOCK_CONST_METHOD0(getWindow, const IWindow&());
        MOCK_METHOD0(getContext, IContext&());
        MOCK_CONST_METHOD0(getContext, const IContext&());

        MOCK_TYPE<ContextMock> contextMock;
        MOCK_TYPE<WindowMock>  windowMock;
    };

    class SurfaceMockWithDestructor : public SurfaceMock< ::testing::StrictMock>
    {
    public:
        SurfaceMockWithDestructor();
        ~SurfaceMockWithDestructor();

        MOCK_METHOD0(Die, void());
    };

    typedef SurfaceMock< ::testing::NiceMock> SurfaceNiceMock;
    typedef SurfaceMock< ::testing::StrictMock> SurfaceStrictMock;
}

#endif
