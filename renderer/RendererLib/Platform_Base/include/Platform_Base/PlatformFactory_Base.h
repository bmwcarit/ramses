//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMFACTORY_BASE_H
#define RAMSES_PLATFORMFACTORY_BASE_H

#include "RendererAPI/IPlatformFactory.h"

namespace ramses_internal
{
    class RendererConfig;
    class IWindowEventHandler;
    class IRenderBackend;

    class PlatformFactory_Base : public IPlatformFactory
    {
    public:
        static IPlatformFactory* CreatePlatformFactory(const RendererConfig& rendererConfig);

        Bool            createPerRendererComponents() override final;
        void            destroyPerRendererComponents() override final;
        IRenderBackend* createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler) override final;
        void            destroyRenderBackend(IRenderBackend& renderBackend)  override final;

        ISystemCompositorController* getSystemCompositorController() const override final;
        const IWindowEventsPollingManager* getWindowEventsPollingManager() const override;

    protected:
        PlatformFactory_Base(const RendererConfig& rendererConfig);
        ~PlatformFactory_Base() override;

        virtual ITextureUploadingAdapter* createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& embeddedCompositor, IWindow& window) override;

        void        destroySystemCompositorController() override final;
        Bool        destroyWindow(IWindow& window) override;
        Bool        destroyContext(IContext& context) override final;
        Bool        destroyDevice(IDevice& context) override final;
        Bool        destroySurface(ISurface& surface) override final;
        Bool        destroyEmbeddedCompositor(IEmbeddedCompositor& compositor) override final;
        Bool        destroyTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter) override final;

        template<typename SYSTEM_COMPOSITOR_CONTROLLER>
        ISystemCompositorController* setPlatformSystemCompositorController(SYSTEM_COMPOSITOR_CONTROLLER*  systemCompositorController);
        template<typename WINDOW>
        IWindow* addPlatformWindow(WINDOW*  window);
        template<typename CONTEXT>
        IContext* addPlatformContext(CONTEXT* context);
        template<typename DEVICE>
        IDevice* addPlatformDevice(DEVICE* device);
        template<typename SURFACE>
        ISurface* addPlatformSurface(SURFACE* surface);
        template<typename EMBEDDED_COMPOSITOR>
        IEmbeddedCompositor* addEmbeddedCompositor(EMBEDDED_COMPOSITOR* compositor);
        template<typename TEXTURE_UPLOADING_ADAPTER>
        ITextureUploadingAdapter* addTextureUploadingAdapter(TEXTURE_UPLOADING_ADAPTER* textureUploadingAdapter);

        template<typename WINDOW>
        WINDOW*  getPlatformWindow(IWindow&  window);
        template<typename CONTEXT>
        CONTEXT* getPlatformContext(IContext& context);
        template<typename DEVICE>
        DEVICE* getPlatformDevice(IDevice& device);
        template<typename SURFACE>
        SURFACE* getPlatformSurface(ISurface& surface);
        template<typename EMBEDDED_COMPOSITOR>
        EMBEDDED_COMPOSITOR* getEmbeddedCompositor(IEmbeddedCompositor& compositor);
        template<typename TEXTURE_UPLOADING_ADAPTER>
        TEXTURE_UPLOADING_ADAPTER* getTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter);

        const RendererConfig& m_rendererConfig;

        ISystemCompositorController*    m_systemCompositorController = nullptr;
        Bool                            m_systemCompositorControllerFailedCreation = false;

        std::vector<IRenderBackend*> m_renderBackends;
        std::vector<IWindow*> m_windows;
        std::vector<IContext*> m_contexts;
        std::vector<IDevice*> m_devices;
        std::vector<ISurface*> m_surfaces;
        std::vector<IEmbeddedCompositor*> m_embeddedCompositors;
        std::vector<ITextureUploadingAdapter*> m_textureUploadingAdapters;
    };

    template <typename WINDOW>
    WINDOW* PlatformFactory_Base::getPlatformWindow(IWindow& window)
    {
        if (contains_c(m_windows, &window))
        {
            return static_cast<WINDOW*>(&window);
        }

        return 0;
    }

    template <typename CONTEXT>
    CONTEXT* PlatformFactory_Base::getPlatformContext(IContext& context)
    {
        if (contains_c(m_contexts, &context))
        {
            return static_cast<CONTEXT*>(&context);
        }

        return 0;
    }

    template <typename DEVICE>
    DEVICE* PlatformFactory_Base::getPlatformDevice(IDevice& device)
    {
        if (contains_c(m_devices, &device))
        {
            return static_cast<DEVICE*>(&device);
        }

        return 0;
    }

    template <typename SURFACE>
    SURFACE* PlatformFactory_Base::getPlatformSurface(ISurface& surface)
    {
        if (contains_c(m_surfaces, &surface))
        {
            return static_cast<SURFACE*>(&surface);
        }

        return 0;
    }

    template <typename EMBEDDED_COMPOSITOR>
    EMBEDDED_COMPOSITOR* PlatformFactory_Base::getEmbeddedCompositor(IEmbeddedCompositor& compositor)
    {
        if (contains_c(m_embeddedCompositors, &compositor))
        {
            return static_cast<EMBEDDED_COMPOSITOR*>(&compositor);
        }

        return 0;
    }

    template <typename TEXTURE_UPLOADING_ADAPTER>
    TEXTURE_UPLOADING_ADAPTER* PlatformFactory_Base::getTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter)
    {
        if (contains_c(m_textureUploadingAdapters, &textureUploadingAdapter))
        {
            return static_cast<TEXTURE_UPLOADING_ADAPTER*>(&textureUploadingAdapter);
        }

        return 0;
    }


    template<typename SYSTEM_COMPOSITOR_CONTROLLER>
    ISystemCompositorController*  PlatformFactory_Base::setPlatformSystemCompositorController(SYSTEM_COMPOSITOR_CONTROLLER* systemCompositorController)
    {
        const Bool initSuccessful = systemCompositorController->init();
        if (initSuccessful)
        {
            m_systemCompositorController = systemCompositorController;
            return systemCompositorController;
        }
        else
        {
            m_systemCompositorControllerFailedCreation = true;

            delete systemCompositorController;
            return nullptr;
        }
    }

    template <typename WINDOW>
    IWindow* PlatformFactory_Base::addPlatformWindow(WINDOW* window)
    {
        const Bool initSuccessful = window->init();
        if (initSuccessful)
        {
            m_windows.push_back(window);
            return window;
        }
        else
        {
            delete window;
            return nullptr;
        }
    }

    template <typename CONTEXT>
    IContext* PlatformFactory_Base::addPlatformContext(CONTEXT* context)
    {
        const Bool initSuccessful = context->init();
        if (initSuccessful)
        {
            m_contexts.push_back(context);
            return context;
        }
        else
        {
            delete context;
            return 0;
        }
    }

    template <typename DEVICE>
    IDevice* PlatformFactory_Base::addPlatformDevice(DEVICE* device)
    {
        const Bool initSuccessful = device->init();
        if (initSuccessful)
        {
            m_devices.push_back(device);
            return device;
        }
        else
        {
            delete device;
            return 0;
        }
    }

    template <typename SURFACE>
    ISurface* PlatformFactory_Base::addPlatformSurface(SURFACE* surface)
    {
        m_surfaces.push_back(surface);
        return surface;
    }

    template <typename EMBEDDED_COMPOSITOR>
    IEmbeddedCompositor* PlatformFactory_Base::addEmbeddedCompositor(EMBEDDED_COMPOSITOR* compositor)
    {
        const Bool initSuccessful = compositor->init();
        if (initSuccessful)
        {
            m_embeddedCompositors.push_back(compositor);
            return compositor;
        }
        else
        {
            delete compositor;
            return 0;
        }
    }

    template <typename TEXTURE_UPLOADING_ADAPTER>
    ITextureUploadingAdapter* PlatformFactory_Base::addTextureUploadingAdapter(TEXTURE_UPLOADING_ADAPTER* textureUploadingAdapter)
    {
        m_textureUploadingAdapters.push_back(textureUploadingAdapter);
        return textureUploadingAdapter;
    }
}

#endif
