//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/Platform_Base.h"
#include "internal/RendererLib/PlatformInterface/IWindow.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IDeviceExtension.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "internal/RendererLib/PlatformInterface/ISystemCompositorController.h"
#include "internal/RendererLib/RenderBackend.h"
#include "internal/RendererLib/ResourceUploadRenderBackend.h"
#include "internal/RendererLib/RendererConfigData.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/RendererLib/PlatformBase/TextureUploadingAdapter_Base.h"
#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    Platform_Base::Platform_Base(RendererConfigData rendererConfig)
        : m_rendererConfig(std::move(rendererConfig))
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

    bool Platform_Base::createDeviceExtension(const DisplayConfigData& displayConfig)
    {
        if(displayConfig.getPlatformRenderNode() != "")
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base::createDeviceExtension: Device extension is not supported for this platform!");
            return false;
        }
        return true;
    }

    IRenderBackend* Platform_Base::createRenderBackend(const DisplayConfigData& displayConfig, IWindowEventHandler& windowEventHandler)
    {
        if (m_rendererConfig.getSystemCompositorControlEnabled() && !createSystemCompositorController())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: will not create render backend components because system compositor controller creation failed");
            return nullptr;
        }

        assert(!m_window);
        if (!createWindow(displayConfig, windowEventHandler))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: window creation failed");
            return nullptr;
        }

        assert(!m_context);
        if (!createContext(displayConfig))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: context creation failed");
            m_window.reset();
            return nullptr;
        }

        m_context->enable();

        assert(!m_deviceExtension);
        if(!createDeviceExtension(displayConfig))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: device extension creation failed");
            m_window.reset();
            m_context.reset();
            return nullptr;
        }

        assert(!m_device);
        if (!createDevice())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: device creation failed");
            m_deviceExtension.reset();
            m_context->disable();
            m_context.reset();
            m_window.reset();
            return nullptr;
        }

        assert(!m_embeddedCompositor);
        if (!createEmbeddedCompositor(displayConfig))
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createRenderBackend: embedded compositor creation failed");
            m_device.reset();
            m_deviceExtension.reset();
            m_context->disable();
            m_context.reset();
            m_window.reset();
            return nullptr;
        }

        assert(!m_textureUploadingAdapter);
        createTextureUploadingAdapter(displayConfig);

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
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy device extension");
        m_deviceExtension.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy context");
        m_context->disable();
        m_context.reset();
        LOG_DEBUG(CONTEXT_RENDERER, "Platform_Base::destroyRenderBackend: destroy window");
        m_window.reset();

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
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createResourceUploadRenderBackend: context creation failed");
            return nullptr;
        }

        m_contextUploading->enable();
        if (!createDeviceUploading())
        {
            LOG_ERROR(CONTEXT_RENDERER, "Platform_Base:createResourceUploadRenderBackend: device creation failed");
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

    void Platform_Base::createTextureUploadingAdapter(const DisplayConfigData& /*unused*/)
    {
        assert(!m_textureUploadingAdapter);
        m_textureUploadingAdapter = std::make_unique<TextureUploadingAdapter_Base>(*m_device);
    }

    bool Platform_Base::createEmbeddedCompositor(const DisplayConfigData& /*unused*/)
    {
        m_embeddedCompositor = std::make_unique<EmbeddedCompositor_Dummy>();
        return m_embeddedCompositor != nullptr;
    }

    bool Platform_Base::createSystemCompositorController()
    {
        return true;
    }
}
