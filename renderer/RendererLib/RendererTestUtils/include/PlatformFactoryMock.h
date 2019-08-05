//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORYMOCK_H
#define RAMSES_PLATFORMFACTORYMOCK_H

#include "renderer_common_gmock_header.h"
#include "SurfaceMock.h"
#include "DeviceMock.h"
#include "EmbeddedCompositorMock.h"
#include "RendererAPI/IPlatformFactory.h"
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
    class PlatformFactoryMock : public IPlatformFactory
    {
    public:
        explicit PlatformFactoryMock(Bool testPerRendererConponents = true);
        ~PlatformFactoryMock() override;


        MOCK_METHOD0(createPerRendererComponents, Bool());
        MOCK_METHOD0(destroyPerRendererComponents, void());
        MOCK_METHOD2(createRenderBackend, IRenderBackend*(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler));
        MOCK_METHOD1(destroyRenderBackend, void(IRenderBackend& renderBackend));

        MOCK_METHOD0(createSystemCompositorController, ISystemCompositorController* ());
        MOCK_METHOD0(destroySystemCompositorController, void());
        MOCK_METHOD2(createWindow, IWindow*(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler));
        MOCK_METHOD1(destroyWindow, Bool(IWindow&));
        MOCK_METHOD1(createContext, IContext*(IWindow& window));
        MOCK_METHOD1(destroyContext, Bool(IContext&));
        MOCK_METHOD1(createDevice, IDevice*(IContext& context));
        MOCK_METHOD1(destroyDevice, Bool(IDevice&));
        MOCK_METHOD2(createSurface, ISurface*(IWindow& window, IContext& context));
        MOCK_METHOD1(destroySurface, Bool(ISurface& surface));
        MOCK_METHOD1(createEmbeddedCompositor, IEmbeddedCompositor*(IContext& context));
        MOCK_METHOD1(destroyEmbeddedCompositor, Bool(IEmbeddedCompositor& compositor));
        MOCK_METHOD3(createTextureUploadingAdapter, ITextureUploadingAdapter*(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window));
        MOCK_METHOD1(destroyTextureUploadingAdapter, Bool(ITextureUploadingAdapter& textureUploadingAdapter));
        MOCK_CONST_METHOD0(getSystemCompositorController, ISystemCompositorController*());
        MOCK_CONST_METHOD0(getWindowEventsPollingManager, const IWindowEventsPollingManager*());

        MOCK_TYPE< SystemCompositorControllerMock > systemCompositorControllerMock;
        MOCK_TYPE< WindowEventsPollingManagerMock > windowEventsPollingManagerMock;
        MOCK_TYPE< RenderBackendMock<MOCK_TYPE> > renderBackendMock;

    private:
        void createDefaultMockCalls(Bool testPerRendererConponents);
    };

    typedef PlatformFactoryMock< ::testing::NiceMock >   PlatformFactoryNiceMock;
    typedef PlatformFactoryMock< ::testing::StrictMock > PlatformFactoryStrictMock;
}

#endif
