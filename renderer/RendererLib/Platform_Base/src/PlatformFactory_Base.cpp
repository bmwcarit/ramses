//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/PlatformFactory_Base.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererAPI/IWindowEventsPollingManager.h"
#include "RendererLib/RenderBackend.h"
#include "RendererLib/RendererConfig.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    PlatformFactory_Base::PlatformFactory_Base(const RendererConfig& rendererConfig)
        : m_rendererConfig(rendererConfig)
    {
    }

    PlatformFactory_Base::~PlatformFactory_Base()
    {
        assert(nullptr == m_systemCompositorController);
        assert(m_renderBackends.empty());
    }

    Bool PlatformFactory_Base::createPerRendererComponents()
    {
        if(m_rendererConfig.getSystemCompositorControlEnabled())
        {
            m_systemCompositorController = createSystemCompositorController();
            if(m_systemCompositorControllerFailedCreation)
            {
                LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createPerRendererComponents:  system compositor controller creation failed");

                return false;
            }
        }

        return true;
    }

    void PlatformFactory_Base::destroyPerRendererComponents()
    {
        destroySystemCompositorController();
    }

    IRenderBackend* PlatformFactory_Base::createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if(m_systemCompositorControllerFailedCreation)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend: will not create display because system compositor controller creation failed");
            return 0;
        }

        IWindow* window = createWindow(displayConfig, windowEventHandler);
        if (0 == window)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend:  window creation failed");
            return 0;
        }

        IContext* context = createContext(*window);
        if (0 == context)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend:  context creation failed");
            destroyWindow(*window);
            return 0;
        }

        ISurface* surface = createSurface(*window, *context);
        if (0 == surface)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend:  window + context creation failed");
            destroyContext(*context);
            destroyWindow(*window);
            return 0;
        }

        // Surface enable is required so that device and texture adapter can load their extrensions
        surface->enable();

        IDevice* device = createDevice(*context);
        if (0 == device)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend:  device creation failed");
            destroySurface(*surface);
            destroyContext(*context);
            destroyWindow(*window);
            return 0;
        }

        IEmbeddedCompositor* embeddedCompositor = createEmbeddedCompositor();

        if (0 == embeddedCompositor)
        {
            LOG_ERROR(CONTEXT_RENDERER, "PlatformFactory_Base:createRenderBackend:  embedded compositor creation failed");
            destroyDevice(*device);
            destroySurface(*surface);
            destroyContext(*context);
            destroyWindow(*window);
            return 0;
        }

        ITextureUploadingAdapter* textureUploadingAdapter = createTextureUploadingAdapter(*device, *embeddedCompositor, *window);

        IRenderBackend* renderBackend = new RenderBackend(*surface, *device, *embeddedCompositor, *textureUploadingAdapter);
        m_renderBackends.push_back(renderBackend);

        return renderBackend;
    }

    void PlatformFactory_Base::destroyRenderBackend(IRenderBackend& renderBackend)
    {
        ISurface& surface = renderBackend.getSurface();
        IWindow& window = surface.getWindow();
        IContext& context = surface.getContext();
        IDevice& device = renderBackend.getDevice();
        IEmbeddedCompositor& embeddedCompositor = renderBackend.getEmbeddedCompositor();
        ITextureUploadingAdapter& textureUploadingAdapter = renderBackend.getTextureUploadingAdapter();

        destroyTextureUploadingAdapter(textureUploadingAdapter);
        destroyEmbeddedCompositor(embeddedCompositor);
        destroyDevice(device);
        destroySurface(surface);
        destroyContext(context);
        destroyWindow(window);

        std::vector<IRenderBackend*>::iterator renderBackendIter = find_c(m_renderBackends, &renderBackend);
        assert(m_renderBackends.end() != renderBackendIter);
        m_renderBackends.erase(renderBackendIter);
        delete &renderBackend;
    }

    ISystemCompositorController* PlatformFactory_Base::getSystemCompositorController() const
    {
        return m_systemCompositorController;
    }

    const IWindowEventsPollingManager* PlatformFactory_Base::getWindowEventsPollingManager() const
    {
        return nullptr;
    }

    ITextureUploadingAdapter* PlatformFactory_Base::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& /*embeddedCompositor*/, IWindow& /*window*/)
    {
        ITextureUploadingAdapter* textureUploadingAdapter = new TextureUploadingAdapter_Base(device);
        return addTextureUploadingAdapter(textureUploadingAdapter);
    }

    Bool ramses_internal::PlatformFactory_Base::destroyWindow(IWindow& window)
    {
        std::vector<IWindow*>::iterator iter = find_c(m_windows, &window);
        if (m_windows.end() != iter)
        {
            IWindow* windowToDelete = *iter;
            m_windows.erase(iter);
            delete windowToDelete;
            return true;
        }

        return false;
    }

    Bool PlatformFactory_Base::destroyContext(IContext& context)
    {
        std::vector<IContext*>::iterator iter = find_c(m_contexts, &context);
        if (m_contexts.end() != iter)
        {
            IContext* contextToDelete = *iter;
            m_contexts.erase(iter);
            delete contextToDelete;
            return true;
        }

        return false;
    }

    Bool PlatformFactory_Base::destroyDevice(IDevice& device)
    {
        std::vector<IDevice*>::iterator iter = find_c(m_devices, &device);
        if (m_devices.end() != iter)
        {
            IDevice* deviceToDelete = *iter;
            m_devices.erase(iter);
            delete deviceToDelete;
            return true;
        }

        return false;
    }

    Bool PlatformFactory_Base::destroySurface(ISurface& surface)
    {
        std::vector<ISurface*>::iterator iter = find_c(m_surfaces, &surface);
        if (m_surfaces.end() != iter)
        {
            ISurface* surfaceToDelete = *iter;
            m_surfaces.erase(iter);
            delete surfaceToDelete;
            return true;
        }

        return false;
    }

    void PlatformFactory_Base::destroySystemCompositorController()
    {
        delete m_systemCompositorController;
        m_systemCompositorController = nullptr;
    }

    Bool PlatformFactory_Base::destroyEmbeddedCompositor(IEmbeddedCompositor& compositor)
    {
        std::vector<IEmbeddedCompositor*>::iterator iter = find_c(m_embeddedCompositors, &compositor);
        if (m_embeddedCompositors.end() != iter)
        {
            IEmbeddedCompositor* compositorToDelete = *iter;
            m_embeddedCompositors.erase(iter);
            delete compositorToDelete;
            return true;
        }

        return false;
    }

    Bool PlatformFactory_Base::destroyTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter)
    {
        auto iter = find_c(m_textureUploadingAdapters, &textureUploadingAdapter);
        if (m_textureUploadingAdapters.end() != iter)
        {
            delete *iter;
            m_textureUploadingAdapters.erase(iter);
            return true;
        }

        return false;
    }
}
