
//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "EmbeddedCompositorMock.h"
#include "DeviceMock.h"
#include "WindowMock.h"
#include "ContextMock.h"
#include "internal/RendererLib/PlatformBase/TextureUploadingAdapter_Base.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    using namespace testing;

    template <template<typename> class MOCK_TYPE>
    class RenderBackendMock : public IRenderBackend
    {
    public:
        RenderBackendMock();
        ~RenderBackendMock() override;
        MOCK_METHOD(IWindow&, getWindow, (), (const, override));
        MOCK_METHOD(IContext&, getContext, (), (const, override));
        MOCK_METHOD(IDevice&, getDevice, (), (const, override));
        MOCK_METHOD(IEmbeddedCompositor&, getEmbeddedCompositor, (), (const, override));
        MOCK_METHOD(ITextureUploadingAdapter&, getTextureUploadingAdapter, (), (const, override));

        MOCK_TYPE< DeviceMock >              deviceMock;
        MOCK_TYPE< WindowMock >              windowMock;
        MOCK_TYPE< ContextMock >             contextMock;
        MOCK_TYPE< EmbeddedCompositorMock >  embeddedCompositorMock;
        TextureUploadingAdapter_Base         textureUploadingAdapter = TextureUploadingAdapter_Base(deviceMock);
    };

    using RenderBackendNiceMock = RenderBackendMock< ::testing::NiceMock>;
    using RenderBackendStrictMock = RenderBackendMock< ::testing::StrictMock>;
}

