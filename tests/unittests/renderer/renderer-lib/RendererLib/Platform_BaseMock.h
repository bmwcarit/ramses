//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/RendererLib/PlatformBase/TextureUploadingAdapter_Base.h"
#include "WindowMock.h"
#include "ContextMock.h"
#include "EmbeddedCompositorMock.h"
#include "SystemCompositorControllerMock.h"
#include "DeviceMock.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    class Platform_BaseMock : public Platform_Base
    {
    public:
        explicit Platform_BaseMock(const RendererConfigData& config);
        ~Platform_BaseMock() override;

        MOCK_METHOD(bool, createWindow, (const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler), (override));
        MOCK_METHOD(bool, createContext, (const DisplayConfigData& displayConfig), (override));
        MOCK_METHOD(bool, createContextUploading, (), (override));
        MOCK_METHOD(bool, createDeviceExtension, (const DisplayConfigData& displayConfig), (override));
        MOCK_METHOD(bool, createDevice, (), (override));
        MOCK_METHOD(bool, createDeviceUploading, (), (override));
        MOCK_METHOD(bool, createEmbeddedCompositor, (const DisplayConfigData& displayConfig), (override));
        MOCK_METHOD(bool, createSystemCompositorController, (), (override));
        MOCK_METHOD(void, createTextureUploadingAdapter, (const DisplayConfigData& displayConfig), (override));

        WindowMock*                     window                     = new ::testing::StrictMock<WindowMock>;
        ContextMock*                    context                    = new ::testing::StrictMock<ContextMock>;
        ContextMock*                    contextUploading           = new ::testing::StrictMock<ContextMock>;
        DeviceMock*                     device                     = new ::testing::StrictMock<DeviceMock>;
        DeviceMock*                     deviceUploading            = new ::testing::StrictMock<DeviceMock>;
        EmbeddedCompositorMock*         embeddedCompositor         = new ::testing::StrictMock<EmbeddedCompositorMock>;
        SystemCompositorControllerMock* systemCompositorController = new ::testing::StrictMock<SystemCompositorControllerMock>;
        TextureUploadingAdapter_Base*   textureUploadingAdapter    = new TextureUploadingAdapter_Base(*device);

    private:
        // ownership of component X goes to platform on createX call, keep rest owned here for proper cleanup
        std::unique_ptr<WindowMock>                     windowOwningPtr{ window };
        std::unique_ptr<ContextMock>                    contextOwningPtr{ context };
        std::unique_ptr<ContextMock>                    contextUploadingOwningPtr{ contextUploading };
        std::unique_ptr<DeviceMock>                     deviceOwningPtr{ device };
        std::unique_ptr<DeviceMock>                     deviceUploadingOwningPtr{ deviceUploading };
        std::unique_ptr<EmbeddedCompositorMock>         embeddedCompositorOwningPtr{ embeddedCompositor };
        std::unique_ptr<SystemCompositorControllerMock> systemCompositorControllerOwningPtr{ systemCompositorController };
        std::unique_ptr<TextureUploadingAdapter_Base>   textureUploadingAdapterOwningPtr{ textureUploadingAdapter };
    };
}
