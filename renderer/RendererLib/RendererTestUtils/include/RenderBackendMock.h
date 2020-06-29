
//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERBACKENDMOCK_H
#define RAMSES_RENDERBACKENDMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IRenderBackend.h"
#include "EmbeddedCompositorMock.h"
#include "DeviceMock.h"
#include "SurfaceMock.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"

namespace ramses_internal
{
    using namespace testing;

    template <template<typename> class MOCK_TYPE>
    class RenderBackendMock : public IRenderBackend
    {
    public:
        RenderBackendMock();
        virtual ~RenderBackendMock();
        MOCK_METHOD(ramses_internal::ISurface&, getSurface, (), (const, override));
        MOCK_METHOD(ramses_internal::IDevice&, getDevice, (), (const, override));
        MOCK_METHOD(ramses_internal::IEmbeddedCompositor&, getEmbeddedCompositor, (), (const, override));
        MOCK_METHOD(ramses_internal::ITextureUploadingAdapter&, getTextureUploadingAdapter, (), (const, override));

        MOCK_TYPE< DeviceMock >              deviceMock;
        MOCK_TYPE< SurfaceMock<MOCK_TYPE> >  surfaceMock;
        MOCK_TYPE< EmbeddedCompositorMock >  embeddedCompositorMock;
        TextureUploadingAdapter_Base                  textureUploadingAdapter = TextureUploadingAdapter_Base(deviceMock);
    };

    typedef RenderBackendMock< ::testing::NiceMock>   RenderBackendNiceMock;
    typedef RenderBackendMock< ::testing::StrictMock> RenderBackendStrictMock;
}
#endif
