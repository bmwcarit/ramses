//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandIVISurface.h"
#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"

#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandIVISurface::WaylandIVISurface(IWaylandClient&             client,
                                         IWaylandResource&           iviApplicationConnectionResource,
                                         WaylandIviSurfaceId         iviSurfaceId,
                                         IWaylandSurface*            surface,
                                         uint32_t                    id,
                                         EmbeddedCompositor_Wayland& compositor)
        : m_iviSurfaceId(iviSurfaceId)
        , m_compositor(compositor)
        , m_clientCredentials(client.getCredentials())
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface");

        if (surface->hasIviSurface())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating ivi-surface : " << iviSurfaceId
                    << ". The wayland surface already has an ivi-surface " << surface->getIviSurfaceId() << "attached!  " << m_clientCredentials);

            iviApplicationConnectionResource.postError(IVI_APPLICATION_ERROR_IVI_ID, "surface already has a ivi-surface");
        }
        else
        {
            if (!m_compositor.hasSurfaceForStreamTexture(iviSurfaceId))
            {
                m_resource = client.resourceCreate(&ivi_surface_interface, iviApplicationConnectionResource.getVersion(), id);
                if (nullptr != m_resource)
                {
                    LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: created succesffully for ivi-surface : " << iviSurfaceId
                            << "  " << m_clientCredentials);

                    m_resource->setImplementation(&m_iviSurfaceInterface, this, ResourceDestroyedCallback);
                    m_surface = surface;
                    m_surface->setIviSurface(this);

                    if (m_surface->getWaylandBuffer() != nullptr)
                    {
                        m_compositor.addToUpdatedStreamTextureSourceIds(iviSurfaceId);
                    }
                }
                else
                {
                    LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating ivi-surface : " << iviSurfaceId
                            << ". Failed creating wayland resource  " << m_clientCredentials);
                    client.postNoMemory();
                }
            }
            else
            {

                const auto credentialsForOtherClient = m_compositor.findSurfaceForStreamTexture(iviSurfaceId).getClientCredentials();

                LOG_ERROR(CONTEXT_RENDERER, "WaylandIVISurface::WaylandIVISurface: failed creating ivi-surface : " << iviSurfaceId
                        << " for  " << m_clientCredentials
                        << ". A wayland surface already eixsts with same ivi-surface id for " << credentialsForOtherClient);

                iviApplicationConnectionResource.postError(IVI_APPLICATION_ERROR_IVI_ID, "ivi-id is already in use");
            }
        }
    }

    bool WaylandIVISurface::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    WaylandIVISurface::~WaylandIVISurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::!WaylandIVISurface: wayland ivi sruface destroyed with ivi-id : " << m_iviSurfaceId
                << "  " << m_clientCredentials);

        if (m_surface != nullptr)
        {
            m_surface->setIviSurface(nullptr);
            m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_iviSurfaceInterface, this, nullptr);
            delete m_resource;
        }
    }

    void WaylandIVISurface::surfaceWasDeleted()
    {
        m_surface = nullptr;
        m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
    }

    void WaylandIVISurface::bufferWasSetToSurface(IWaylandBuffer* buffer)
    {
        if (buffer != nullptr)
        {
            m_compositor.addToUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }
        else
        {
            m_compositor.removeFromUpdatedStreamTextureSourceIds(m_iviSurfaceId);
        }
    }

    void WaylandIVISurface::resourceDestroyed()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::resourceDestroyed");

        // wl_resource is destroyed outside by the Wayland library, so m_resource looses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }


    void WaylandIVISurface::IVISurfaceDestroyCallback(wl_client* client, wl_resource* iviSurfaceResource)
    {
        UNUSED(client)
        WaylandIVISurface* iviSurface = static_cast<WaylandIVISurface*>(wl_resource_get_user_data(iviSurfaceResource));
        delete iviSurface;
    }

    void WaylandIVISurface::ResourceDestroyedCallback(wl_resource* iviSurfaceResource)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVISurface::IVISurfaceDestroyedCallback");

        WaylandIVISurface* iviSurface = static_cast<WaylandIVISurface*>(wl_resource_get_user_data(iviSurfaceResource));
        iviSurface->resourceDestroyed();
        delete iviSurface;
    }

    WaylandIviSurfaceId WaylandIVISurface::getIviId() const
    {
        return m_iviSurfaceId;
    }
};
