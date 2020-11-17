//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMMOCK_H
#define RAMSES_PLATFORMMOCK_H

#include "renderer_common_gmock_header.h"
#include "SurfaceMock.h"
#include "DeviceMock.h"
#include "EmbeddedCompositorMock.h"
#include "RendererAPI/IPlatform.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererLib/DisplayConfig.h"

#include "RenderBackendMock.h"
#include "SystemCompositorControllerMock.h"
#include "WindowEventsPollingManagerMock.h"

namespace ramses_internal
{
    class RendererConfig;

    template <template<typename> class MOCK_TYPE>
    class PlatformMock : public IPlatform
    {
    public:
        explicit PlatformMock(Bool testPerRendererConponents = true);
        ~PlatformMock() override;


        MOCK_METHOD(Bool, createPerRendererComponents, (), (override));
        MOCK_METHOD(void, destroyPerRendererComponents, (), (override));
        MOCK_METHOD(IRenderBackend*, createRenderBackend, (const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(void, destroyRenderBackend, (IRenderBackend& renderBackend), (override));

        MOCK_METHOD(ISystemCompositorController* , createSystemCompositorController, (), (override));
        MOCK_METHOD(void, destroySystemCompositorController, (), (override));
        MOCK_METHOD(IWindow*, createWindow, (const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(Bool, destroyWindow, (IWindow&), (override));
        MOCK_METHOD(IContext*, createContext, (IWindow& window), (override));
        MOCK_METHOD(Bool, destroyContext, (IContext&), (override));
        MOCK_METHOD(IDevice*, createDevice, (IContext& context), (override));
        MOCK_METHOD(Bool, destroyDevice, (IDevice&), (override));
        MOCK_METHOD(ISurface*, createSurface, (IWindow& window, IContext& context), (override));
        MOCK_METHOD(Bool, destroySurface, (ISurface& surface), (override));
        MOCK_METHOD(IEmbeddedCompositor*, createEmbeddedCompositor,(const DisplayConfig& displayConfig, IContext& context), (override));
        MOCK_METHOD(Bool, destroyEmbeddedCompositor, (IEmbeddedCompositor& compositor), (override));
        MOCK_METHOD(ITextureUploadingAdapter*, createTextureUploadingAdapter, (IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window), (override));
        MOCK_METHOD(Bool, destroyTextureUploadingAdapter, (ITextureUploadingAdapter& textureUploadingAdapter), (override));
        MOCK_METHOD(ISystemCompositorController*, getSystemCompositorController, (), (const, override));
        MOCK_METHOD(const IWindowEventsPollingManager*, getWindowEventsPollingManager, (), (const, override));

        MOCK_TYPE< SystemCompositorControllerMock > systemCompositorControllerMock;
        MOCK_TYPE< WindowEventsPollingManagerMock > windowEventsPollingManagerMock;
        MOCK_TYPE< RenderBackendMock<MOCK_TYPE> > renderBackendMock;

    private:
        void createDefaultMockCalls(Bool testPerRendererConponents);
    };

    using PlatformNiceMock = PlatformMock< ::testing::NiceMock>;
    using PlatformStrictMock = PlatformMock< ::testing::StrictMock>;
}

#endif
