//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEUPLOADRENDERBACKENDMOCK_H
#define RAMSES_RESOURCEUPLOADRENDERBACKENDMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererAPI/IResourceUploadRenderBackend.h"
#include "DeviceMock.h"
#include "ContextMock.h"

namespace ramses_internal
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

        virtual ~ResourceUploadRenderBackendMock(){}
        MOCK_METHOD(ramses_internal::IContext&, getContext, (), (const, override));
        MOCK_METHOD(ramses_internal::IDevice&, getDevice, (), (const, override));

        MOCK_TYPE< DeviceMock >              deviceMock;
        MOCK_TYPE< ContextMock >             contextMock;
    };

    using ResourceUploadRenderBackendNiceMock = ResourceUploadRenderBackendMock< ::testing::NiceMock>;
    using ResourceUploadRenderBackendStrictMock = ResourceUploadRenderBackendMock< ::testing::StrictMock>;
}
#endif
