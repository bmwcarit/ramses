//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/Platform_Base.h"
#include "RendererAPI/IWindow.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererAPI/IWindowEventsPollingManager.h"
#include "RendererLib/RenderBackend.h"
#include "RendererLib/ResourceUploadRenderBackend.h"
#include "RendererLib/RendererConfig.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "Platform_Base/Surface.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    Platform_Base::Platform_Base(const RendererConfig& rendererConfig)
        : m_rendererConfig(rendererConfig)
    {
    }

    Platform_Base::~Platform_Base()
    {
        assert(nullptr == m_systemCompositorController);
        assert(m_renderBackends.empty());
        assert(m_resourceUploadRenderBackends.empty());
    }

    Bool Platform_Base::createPerRendererComponents()
    {
        if(m_rendererConfig.getSystemCompositorControlEnabled())
        {
            m_systemCompositorController = createSystemCompositorController();
            if(m_systemCompositorControllerFailedCreation)
            {
                LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createPerRendererComponents:  system compositor controller creation failed");

                return false;
            }
        }

        return true;
    }

    void Platform_Base::destroyPerRendererComponents()
    {
        destroySystemCompositorController();
    }

    IRenderBackend* Platform_Base::createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if(m_systemCompositorControllerFailedCreation)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: will not create display because system compositor controller creation failed");
            return nullptr;
        }

        IWindow* window = createWindow(displayConfig, windowEventHandler);
        if (nullptr == window)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend:  window creation failed");
            return nullptr;
        }

        IContext* context = createContext(displayConfig, *window);
        if (nullptr == context)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend:  context creation failed");
            destroyWindow(*window);
            return nullptr;
        }

        ISurface* surface = createSurface(*window, *context);
        if (nullptr == surface)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend:  window + context creation failed");
            destroyContext(*context);
            destroyWindow(*window);
            return nullptr;
        }

        // Surface enable is required so that device and texture adapter can load their extensions
        surface->enable();

        IDevice* device = createDevice(*context);
        if (nullptr == device)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend:  device creation failed");
            destroySurface(*surface);
            destroyContext(*context);
            destroyWindow(*window);
            return nullptr;
        }

        IEmbeddedCompositor* embeddedCompositor = createEmbeddedCompositor(displayConfig, *context);

        if (nullptr == embeddedCompositor)
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend:  embedded compositor creation failed");
            destroyDevice(*device);
            destroySurface(*surface);
            destroyContext(*context);
            destroyWindow(*window);
            return nullptr;
        }

        ITextureUploadingAdapter* textureUploadingAdapter = createTextureUploadingAdapter(*device, *embeddedCompositor, *window);

        IRenderBackend* renderBackend = new RenderBackend(*surface, *device, *embeddedCompositor, *textureUploadingAdapter);
        m_renderBackends.push_back(renderBackend);

        return renderBackend;
    }

    void Platform_Base::destroyRenderBackend(IRenderBackend& renderBackend)
    {
        ISurface& surface = renderBackend.getSurface();
        IWindow& window = surface.getWindow();
        IContext& context = surface.getContext();
        IDevice& device = renderBackend.getDevice();
        IEmbeddedCompositor& embeddedCompositor = renderBackend.getEmbeddedCompositor();
        ITextureUploadingAdapter& textureUploadingAdapter = renderBackend.getTextureUploadingAdapter();

        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy texture uploadadapter");
        destroyTextureUploadingAdapter(textureUploadingAdapter);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy embeddedcompositor");
        destroyEmbeddedCompositor(embeddedCompositor);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy device");
        destroyDevice(device);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy surface");
        destroySurface(surface);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy context");
        destroyContext(context);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy window");
        destroyWindow(window);

        std::vector<IRenderBackend*>::iterator renderBackendIter = find_c(m_renderBackends, &renderBackend);
        assert(m_renderBackends.end() != renderBackendIter);
        m_renderBackends.erase(renderBackendIter);
        delete &renderBackend;
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: done.");
    }

    IResourceUploadRenderBackend* Platform_Base::createResourceUploadRenderBackend(const IRenderBackend& mainRenderBackend)
    {
        IWindow& window = mainRenderBackend.getSurface().getWindow();
        IContext& mainContext = mainRenderBackend.getSurface().getContext();
        DisplayConfig dummyDisplayConfig;
        auto context = createContext(dummyDisplayConfig, window, &mainContext);
        if(!context)
            return nullptr;

        context->enable();
        auto device = createDevice(*context);

        if(!device)
        {
            context->disable();
            destroyContext(*context);
            return nullptr;
        }

        IResourceUploadRenderBackend* renderBackend = new ResourceUploadRenderBackend(*context, *device);
        m_resourceUploadRenderBackends.push_back(renderBackend);

        return renderBackend;
    }

    void Platform_Base::destroyResourceUploadRenderBackend(IResourceUploadRenderBackend& renderBackend)
    {
        IContext& context = renderBackend.getContext();
        IDevice& device = renderBackend.getDevice();

        destroyDevice(device);
        context.disable();
        destroyContext(context);

        std::vector<IResourceUploadRenderBackend*>::iterator renderBackendIter = find_c(m_resourceUploadRenderBackends, &renderBackend);
        assert(m_resourceUploadRenderBackends.end() != renderBackendIter);
        m_resourceUploadRenderBackends.erase(renderBackendIter);
        delete &renderBackend;
    }

    ISystemCompositorController* Platform_Base::getSystemCompositorController() const
    {
        return m_systemCompositorController;
    }

    const IWindowEventsPollingManager* Platform_Base::getWindowEventsPollingManager() const
    {
        return nullptr;
    }

    ITextureUploadingAdapter* Platform_Base::createTextureUploadingAdapter(IDevice& device, IEmbeddedCompositor& /*embeddedCompositor*/, IWindow& /*window*/)
    {
        ITextureUploadingAdapter* textureUploadingAdapter = new TextureUploadingAdapter_Base(device);
        return addTextureUploadingAdapter(textureUploadingAdapter);
    }

    ISurface* Platform_Base::createSurface(IWindow& window, IContext& context)
    {
        Surface* platformSurface = new Surface(window, context);
        return addPlatformSurface(platformSurface);
    }

    Bool ramses_internal::Platform_Base::destroyWindow(IWindow& window)
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

    Bool Platform_Base::destroyContext(IContext& context)
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

    Bool Platform_Base::destroyDevice(IDevice& device)
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

    Bool Platform_Base::destroySurface(ISurface& surface)
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

    void Platform_Base::destroySystemCompositorController()
    {
        delete m_systemCompositorController;
        m_systemCompositorController = nullptr;
    }

    Bool Platform_Base::destroyEmbeddedCompositor(IEmbeddedCompositor& compositor)
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

    Bool Platform_Base::destroyTextureUploadingAdapter(ITextureUploadingAdapter& textureUploadingAdapter)
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
