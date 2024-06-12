//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IPlatform.h"
#include "RenderBackendMock.h"
#include "SystemCompositorControllerMock.h"
#include "ResourceUploadRenderBackendMock.h"

namespace ramses::internal
{
    class RendererConfigData;

    template <template<typename> class MOCK_TYPE>
    class PlatformMock : public IPlatform
    {
    public:
        explicit PlatformMock();
        ~PlatformMock() override;

        MOCK_METHOD(IRenderBackend*, createRenderBackend, (const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(void, destroyRenderBackend, (), (override));
        MOCK_METHOD(IResourceUploadRenderBackend*, createResourceUploadRenderBackend, (), (override));
        MOCK_METHOD(void, destroyResourceUploadRenderBackend, (), (override));
        MOCK_METHOD(ISystemCompositorController*, getSystemCompositorController, (), (override));

        MOCK_TYPE<RenderBackendMock<MOCK_TYPE>>     renderBackendMock;
        MOCK_TYPE<ResourceUploadRenderBackendMock<MOCK_TYPE>> resourceUploadRenderBackendMock;
        MOCK_TYPE<SystemCompositorControllerMock>   systemCompositorControllerMock;
    };

    using PlatformNiceMock = PlatformMock< ::testing::NiceMock>;
    using PlatformStrictMock = PlatformMock< ::testing::StrictMock>;
}
