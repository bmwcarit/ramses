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
#include "RendererAPI/IEmbeddedCompositor.h"
#include "RendererAPI/ISystemCompositorController.h"
#include "RendererAPI/IWindowEventsPollingManager.h"
#include "RendererLib/RenderBackend.h"
#include "RendererLib/ResourceUploadRenderBackend.h"
#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"
#include "Platform_Base/TextureUploadingAdapter_Base.h"
#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "Utils/ThreadLocalLog.h"

namespace ramses_internal
{
    Platform_Base::Platform_Base(const RendererConfig& rendererConfig)
        : m_rendererConfig(rendererConfig)
    {
    }

    Platform_Base::~Platform_Base()
    {
        assert(!m_systemCompositorController);
        assert(!m_embeddedCompositor);
        assert(!m_window);
        assert(!m_renderBackend);
        assert(!m_resourceUploadRenderBackend);
        assert(!m_textureUploadingAdapter);
    }

    IRenderBackend* Platform_Base::createRenderBackend(const DisplayConfig& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if (m_rendererConfig.getSystemCompositorControlEnabled() && !createSystemCompositorController())
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: will not create render backend components because system compositor controller creation failed");
            return nullptr;
        }

        assert(!m_window);
        if (!createWindow(displayConfig, windowEventHandler))
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: window creation failed");
            return nullptr;
        }

        assert(!m_context);
        if (!createContext(displayConfig))
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: context creation failed");
            destroyWindow();
            return nullptr;
        }

        m_context->enable();

        assert(!m_device);
        if (!createDevice())
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: device creation failed");
            m_context->disable();
            m_context.reset();
            destroyWindow();
            return nullptr;
        }

        assert(!m_embeddedCompositor);
        if (!createEmbeddedCompositor(displayConfig))
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: embedded compositor creation failed");
            m_device.reset();
            m_context->disable();
            m_context.reset();
            destroyWindow();
            return nullptr;
        }

        assert(!m_textureUploadingAdapter);
        createTextureUploadingAdapter();

        assert(!m_renderBackend);
        m_renderBackend = std::make_unique<RenderBackend>(*m_window, *m_context, *m_device, *m_embeddedCompositor, *m_textureUploadingAdapter);

        return m_renderBackend.get();
    }

    void Platform_Base::destroyRenderBackend()
    {
        assert(m_renderBackend);
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy texture uploadadapter");
        m_textureUploadingAdapter.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy embeddedcompositor");
        m_embeddedCompositor.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy device");
        m_device.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy context");
        m_context->disable();
        m_context.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy window");
        destroyWindow();

        m_renderBackend.reset();

        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy system compositor");
        m_systemCompositorController.reset();

        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: done.");
    }

    IResourceUploadRenderBackend* Platform_Base::createResourceUploadRenderBackend()
    {
        assert(!m_contextUploading);
        if (!createContextUploading())
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createResourceUploadRenderBackend: context creation failed");
            return nullptr;
        }

        m_contextUploading->enable();
        if (!createDeviceUploading())
        {
            LOG_ERROR_R(CONTEXT_RENDERER, "Platform_Base:createResourceUploadRenderBackend: device creation failed");
            m_contextUploading->disable();
            m_contextUploading.reset();
            return nullptr;
        }

        assert(!m_resourceUploadRenderBackend);
        m_resourceUploadRenderBackend = std::make_unique<ResourceUploadRenderBackend>(*m_contextUploading, *m_deviceUploading);

        return m_resourceUploadRenderBackend.get();
    }

    void Platform_Base::destroyResourceUploadRenderBackend()
    {
        assert(m_resourceUploadRenderBackend);
        m_deviceUploading.reset();
        m_contextUploading->disable();
        m_contextUploading.reset();
        m_resourceUploadRenderBackend.reset();
    }

    ISystemCompositorController* Platform_Base::getSystemCompositorController()
    {
        return m_systemCompositorController.get();
    }

    const IWindowEventsPollingManager* Platform_Base::getWindowEventsPollingManager() const
    {
        return nullptr;
    }

    void Platform_Base::createTextureUploadingAdapter()
    {
        assert(!m_textureUploadingAdapter);
        m_textureUploadingAdapter = std::make_unique<TextureUploadingAdapter_Base>(*m_device);
    }

    bool Platform_Base::createEmbeddedCompositor(const DisplayConfig&)
    {
        auto embeddedCompositor = std::make_unique<EmbeddedCompositor_Dummy>();
        if (embeddedCompositor->init())
            m_embeddedCompositor = std::move(embeddedCompositor);

        return m_embeddedCompositor != nullptr;
    }

    bool Platform_Base::createSystemCompositorController()
    {
        return true;
    }

    void Platform_Base::destroyWindow()
    {
        m_window.reset();
    }
}
