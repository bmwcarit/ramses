//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_BASEMOCK_H
#define RAMSES_PLATFORM_BASEMOCK_H

#include "gmock/gmock.h"
#include "renderer_common_gmock_header.h"
#include "Platform_Base/Platform_Base.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "RendererLib/RendererConfig.h"
#include "WindowMock.h"
#include "ContextMock.h"
#include "EmbeddedCompositorMock.h"
#include "SystemCompositorControllerMock.h"
#include "DeviceMock.h"
#include "WindowEventsPollingManagerMock.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"

namespace ramses_internal
{
    class Platform_BaseMock : public Platform_Base
    {
    public:
        explicit Platform_BaseMock(const RendererConfig& config);
        virtual ~Platform_BaseMock() override;

        template <typename MockT>
        MockT* createMockObjectHelper()
        {
            MockT* mockObject = new ::testing::StrictMock<MockT>();
            return mockObject;
        }

        void createRenderBackendMockObjects();

        MOCK_METHOD(ISystemCompositorController* , createSystemCompositorController, (), (override));
        MOCK_METHOD(IWindow*, createWindow, (const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(IContext*, createContext, (const DisplayConfig& displayConfig, IWindow& window, IContext*), (override));
        MOCK_METHOD(IDevice*, createDevice, (IContext& context), (override));
        MOCK_METHOD(IEmbeddedCompositor*, createEmbeddedCompositor, (const DisplayConfig& displayConfig, IContext& context), (override));
        MOCK_METHOD(ITextureUploadingAdapter*, createTextureUploadingAdapter, (IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window), (override));

        WindowMockWithDestructor*                     window                  = nullptr;
        ContextMockWithDestructor*                    context                 = nullptr;
        DeviceMockWithDestructor*                     device                  = nullptr;
        EmbeddedCompositorMockWithDestructor*         embeddedCompositor      = nullptr;
        ::testing::StrictMock<DeviceMock>             deviceMock;
        TextureUploadingAdapter_Base                  textureUploadingAdapter = TextureUploadingAdapter_Base(deviceMock);
        SystemCompositorControllerMockWithDestructor* systemCompositorController = nullptr;

    private:
        ISystemCompositorController* createSystemCompositorController_fake()
        {
            return setPlatformSystemCompositorController(systemCompositorController);
        }

        IWindow* createWindow_fake(const DisplayConfig& /*displayConfig*/, IWindowEventHandler& /*windowEventHandler*/)
        {
            return addPlatformWindow(window);
        }

        IDevice* createDevice_fake(IContext& /*context*/)
        {
            return addPlatformDevice(device);
        }

        IContext* createContext_fake(const DisplayConfig&, IWindow& /*window*/, IContext*)
        {
            return addPlatformContext(context);
        }

        IEmbeddedCompositor* createEmbeddedCompositor_fake(const DisplayConfig&, IContext& /*context*/)
        {
            return addEmbeddedCompositor(embeddedCompositor);
        }

        ITextureUploadingAdapter* createTextureUploadingAdapter_fake(IDevice& /*device*/, IEmbeddedCompositor& /*embeddedCompositor*/, IWindow& /*window*/)
        {
            return &textureUploadingAdapter;
        }
    };
}

#endif
