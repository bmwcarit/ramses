//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_BASEMOCK_H
#define RAMSES_PLATFORMFACTORY_BASEMOCK_H

#include "gmock/gmock.h"
#include "renderer_common_gmock_header.h"
#include "Platform_Base/PlatformFactory_Base.h"
#include "RendererAPI/IWindowEventHandler.h"
#include "RendererLib/RendererConfig.h"
#include "WindowMock.h"
#include "ContextMock.h"
#include "SurfaceMock.h"
#include "EmbeddedCompositorMock.h"
#include "SystemCompositorControllerMock.h"
#include "DeviceMock.h"
#include "WindowEventsPollingManagerMock.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"

namespace ramses_internal
{
    class PlatformFactory_BaseMock : public PlatformFactory_Base
    {
    public:
        explicit PlatformFactory_BaseMock(const RendererConfig& config);
        virtual ~PlatformFactory_BaseMock();

        template <typename MockT>
        MockT* createMockObjectHelper()
        {
            MockT* mockObject = new ::testing::StrictMock<MockT>();
            return mockObject;
        }

        void createRenderBackendMockObjects();

        MOCK_METHOD(ISystemCompositorController* , createSystemCompositorController, (), (override));
        MOCK_METHOD(IWindow*, createWindow, (const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(IContext*, createContext, (IWindow& window), (override));
        MOCK_METHOD(IDevice*, createDevice, (IContext& context), (override));
        MOCK_METHOD(ISurface*, createSurface, (IWindow& window, IContext& context), (override));
        MOCK_METHOD(IEmbeddedCompositor*, createEmbeddedCompositor, (const DisplayConfig& displayConfig, IContext& context), (override));
        MOCK_METHOD(ITextureUploadingAdapter*, createTextureUploadingAdapter, (IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window), (override));

        WindowMockWithDestructor*                     window                  = nullptr;
        ContextMockWithDestructor*                    context                 = nullptr;
        SurfaceMockWithDestructor*                    surface                 = nullptr;
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

        IContext* createContext_fake(IWindow& /*window*/)
        {
            return addPlatformContext(context);
        }

        ISurface* createSurface_fake(IWindow& /*window*/, IContext& /*context*/)
        {
            return addPlatformSurface(surface);
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
