//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_BASE_H
#define RAMSES_PLATFORM_BASE_H

#include "RendererAPI/IPlatform.h"
#include "RendererLib/RendererConfig.h"
#include "Collections/Vector.h"
#include <vector>
#include <memory>

namespace ramses_internal
{
    class ITextureUploadingAdapter;
    class IDevice;
    class IEmbeddedCompositor;
    class IWindow;
    class IContext;

    class Platform_Base : public IPlatform
    {
    public:
        static IPlatform* CreatePlatform(const RendererConfig& rendererConfig);

        virtual IRenderBackend* createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        virtual void            destroyRenderBackend()  override final;

        virtual IResourceUploadRenderBackend* createResourceUploadRenderBackend() override final;
        virtual void                          destroyResourceUploadRenderBackend() override final;

        virtual ISystemCompositorController* getSystemCompositorController() override;
        virtual const IWindowEventsPollingManager* getWindowEventsPollingManager() const override;

    protected:
        explicit Platform_Base(const RendererConfig& rendererConfig);
        virtual ~Platform_Base() override;

        virtual bool createWindow(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) = 0;
        virtual void destroyWindow();
        virtual bool createContext(const DisplayConfig& displayConfig) = 0;
        virtual bool createContextUploading() = 0;
        virtual bool createDevice() = 0;
        virtual bool createDeviceUploading() = 0;
        virtual bool createEmbeddedCompositor(const DisplayConfig& displayConfig);
        virtual void createTextureUploadingAdapter();
        virtual bool createSystemCompositorController();

        RendererConfig m_rendererConfig;

        std::unique_ptr<IRenderBackend> m_renderBackend;
        std::unique_ptr<IResourceUploadRenderBackend> m_resourceUploadRenderBackend;
        std::unique_ptr<IWindow> m_window;
        std::unique_ptr<IContext> m_context;
        std::unique_ptr<IContext> m_contextUploading;
        std::unique_ptr<IDevice> m_device;
        std::unique_ptr<IDevice> m_deviceUploading;
        std::unique_ptr<ISystemCompositorController> m_systemCompositorController;
        std::unique_ptr<IEmbeddedCompositor> m_embeddedCompositor;
        std::unique_ptr<ITextureUploadingAdapter> m_textureUploadingAdapter;
    };
}

#endif
