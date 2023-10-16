//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IPlatform.h"
#include "internal/RendererLib/RendererConfig.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include <vector>
#include <memory>

namespace ramses::internal
{
    class ITextureUploadingAdapter;
    class IDevice;
    class IDeviceExtension;
    class IEmbeddedCompositor;
    class IWindow;
    class IContext;

    class Platform_Base : public IPlatform
    {
    public:
        IRenderBackend* createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) final override;
        void            destroyRenderBackend()  final override;

        IResourceUploadRenderBackend* createResourceUploadRenderBackend() final override;
        void                          destroyResourceUploadRenderBackend() final override;

        ISystemCompositorController* getSystemCompositorController() override;

    protected:
        explicit Platform_Base(RendererConfig rendererConfig);
        ~Platform_Base() override;

        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) = 0;
        virtual bool createContext(const DisplayConfig& displayConfig) = 0;
        virtual bool createContextUploading() = 0;
        virtual bool createDeviceExtension(const DisplayConfig& displayConfig);
        virtual bool createDevice() = 0;
        virtual bool createDeviceUploading() = 0;
        virtual bool createEmbeddedCompositor(const DisplayConfig& displayConfig);
        virtual void createTextureUploadingAdapter(const DisplayConfig& displayConfig);
        virtual bool createSystemCompositorController();

        RendererConfig m_rendererConfig;

        std::unique_ptr<IRenderBackend> m_renderBackend;
        std::unique_ptr<IResourceUploadRenderBackend> m_resourceUploadRenderBackend;
        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IContext> m_context;
        std::unique_ptr<IContext> m_contextUploading;
        std::unique_ptr<IDeviceExtension> m_deviceExtension;
        std::unique_ptr<IDevice> m_device;
        std::unique_ptr<IDevice> m_deviceUploading;
        std::unique_ptr<ISystemCompositorController> m_systemCompositorController;
        std::unique_ptr<IEmbeddedCompositor> m_embeddedCompositor;
        std::unique_ptr<ITextureUploadingAdapter> m_textureUploadingAdapter;
    };
}
