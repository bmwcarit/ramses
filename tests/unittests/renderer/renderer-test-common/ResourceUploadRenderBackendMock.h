//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IResourceUploadRenderBackend.h"
#include "DeviceMock.h"
#include "ContextMock.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    using namespace testing;

    template <template<typename> class MOCK_TYPE>
    class ResourceUploadRenderBackendMock : public IResourceUploadRenderBackend
    {
    public:
        ResourceUploadRenderBackendMock()
        {
            ON_CALL(*this, getContext()).WillByDefault(ReturnRef(contextMock));
            ON_CALL(*this, getDevice()).WillByDefault(ReturnRef(deviceMock));

            EXPECT_CALL(*this, getDevice()).Times(AnyNumber());
            EXPECT_CALL(*this, getContext()).Times(AnyNumber());
        }

        ~ResourceUploadRenderBackendMock() override = default;
        MOCK_METHOD(IContext&, getContext, (), (const, override));
        MOCK_METHOD(IDevice&, getDevice, (), (const, override));

        MOCK_TYPE< DeviceMock >              deviceMock;
        MOCK_TYPE< ContextMock >             contextMock;
    };

    using ResourceUploadRenderBackendNiceMock = ResourceUploadRenderBackendMock< ::testing::NiceMock>;
    using ResourceUploadRenderBackendStrictMock = ResourceUploadRenderBackendMock< ::testing::StrictMock>;
}

