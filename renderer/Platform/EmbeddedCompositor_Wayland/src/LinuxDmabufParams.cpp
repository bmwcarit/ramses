//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <drm_fourcc.h>

#include "wayland-server.h"

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufParams.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufBuffer.h"
#include "TextureUploadingAdapter_Wayland/LinuxDmabuf.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"
#include <assert.h>

namespace ramses_internal
{
    struct zwp_linux_buffer_params_v1_interface const LinuxDmabufParams::m_paramsInterface =
    {
        .destroy = LinuxDmabufParams::DmabufParamsDestroyCallback,
        .add = LinuxDmabufParams::DmabufParamsAddCallback,
        .create = LinuxDmabufParams::DmabufParamsCreateCallback,
        .create_immed = LinuxDmabufParams::DmabufParamsCreateImmedCallback,
    };

    LinuxDmabufParams::LinuxDmabufParams(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        m_data.reset(new LinuxDmabufBufferData());
        m_resource.reset(client.resourceCreate(&zwp_linux_buffer_params_v1_interface, version, id));

        if (m_resource)
        {
            m_resource->setImplementation(&m_paramsInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinuxDmabufConnection::LinuxDmabufConnection(): Could not create wayland resource");
            client.postNoMemory();
        }
    }

    LinuxDmabufParams::~LinuxDmabufParams()
    {
        if (m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_paramsInterface, this, nullptr);
        }
    }

    bool LinuxDmabufParams::wasSuccessfullyInitialized() const
    {
        // This looks stupid, but the std::unique_ptr bool() operator won't directly convert
        // to the return-value.
        return m_resource ? true : false;
    }

    void LinuxDmabufParams::ResourceDestroyedCallback(wl_resource* dmabufParamsResource)
    {
        LinuxDmabufParams* dmabufParams = static_cast<LinuxDmabufParams*>(wl_resource_get_user_data(dmabufParamsResource));

        dmabufParams->resourceDestroyed();
        delete dmabufParams;
    }

    void LinuxDmabufParams::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "LinuxDmabufParams::resourceDestroyed");
        assert(m_resource);

        // wl_resource is destroyed outside by the Wayland library, so our job here is to abandon
        // ownership of the Wayland resource so that we don't call wl_resource_destroy().
        m_resource->disownWaylandResource();
    }

    void LinuxDmabufParams::DmabufParamsDestroyCallback(wl_client* client, wl_resource* dmabufParamsResource)
    {
        UNUSED(client)

        LinuxDmabufParams* params = static_cast<LinuxDmabufParams*>(wl_resource_get_user_data(dmabufParamsResource));
        delete params;
    }

    void LinuxDmabufParams::DmabufParamsAddCallback(wl_client* client, wl_resource* dmabufParamsResource, int32_t fd, uint32_t plane_idx, uint32_t offset, uint32_t stride, uint32_t modifier_hi, uint32_t modifier_lo)
    {
        UNUSED(client)
        LinuxDmabufParams* params = static_cast<LinuxDmabufParams*>(wl_resource_get_user_data(dmabufParamsResource));
        params->addPlane(fd, plane_idx, offset, stride, modifier_hi, modifier_lo);
    }

    void LinuxDmabufParams::DmabufParamsCreateCallback(wl_client* client, wl_resource* dmabufParamsResource, int32_t width, int32_t height, uint32_t format, uint32_t flags)
    {
        WaylandClient waylandClient(client);
        LinuxDmabufParams* params = static_cast<LinuxDmabufParams*>(wl_resource_get_user_data(dmabufParamsResource));

        params->createBuffer(waylandClient, 0 /* allocate an available wl_resource ID */, width, height, format, flags);
        params->m_createInvoked = true;
    }

    void LinuxDmabufParams::DmabufParamsCreateImmedCallback(wl_client* client, wl_resource* dmabufParamsResource, uint32_t buffer_id, int32_t width, int32_t height, uint32_t format, uint32_t flags)
    {
        WaylandClient waylandClient(client);
        LinuxDmabufParams* params = static_cast<LinuxDmabufParams*>(wl_resource_get_user_data(dmabufParamsResource));

        params->createBuffer(waylandClient, buffer_id, width, height, format, flags);
        params->m_createInvoked = true;
    }

    void LinuxDmabufParams::addPlane(int32_t fd, uint32_t index, uint32_t offset, uint32_t stride, uint32_t modifier_hi, uint32_t modifier_lo)
    {
        if (m_createInvoked)
        {
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED, "params was already used to create a wl_buffer");
            ::close(fd);
            return;
        }

        if (index >= LinuxDmabufBufferData::MAX_DMABUF_PLANES)
        {
            StringOutputStream message;
            message << "plane index " << index << " is too high";
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX, message.release());
            ::close(fd);
            return;
        }

        if (m_data->isPlaneDataSet(index))
        {
            StringOutputStream message;
            message << "a dmabuf has already been added for plane " << index;
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET, message.release());
            ::close(fd);
            return;
        }

        PUSH_DISABLE_C_STYLE_CAST_WARNING

        uint64_t const modifier = (m_resource->getVersion() < ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION)
                    ? DRM_FORMAT_MOD_INVALID
                    : (static_cast<uint64_t>(modifier_hi) << 32) | modifier_lo;

        POP_DISABLE_C_STYLE_CAST_WARNING

        m_data->setPlaneData(index, fd, offset, stride, modifier);
    }

    struct wl_buffer_interface const LinuxDmabufBuffer::m_bufferInterface =
    {
        .destroy = LinuxDmabufBuffer::DmabufBufferDestroyCallback,
    };

    static void BufferDestroyCallback(wl_resource* bufferResource)
    {
        LinuxDmabufBufferData* data = static_cast<LinuxDmabufBufferData*>(wl_resource_get_user_data(bufferResource));
        assert(nullptr != data);
        delete data;
    }

    void LinuxDmabufParams::createBuffer(IWaylandClient& client, uint32_t bufferId, int32_t width, int32_t height, uint32_t format, uint32_t flags)
    {
        if (m_createInvoked)
        {
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED, "params was already used to create a wl_buffer");
            return;
        }

        if (m_data->getNumPlanes() < 1)
        {
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE, "no dmabuf has been added to the params");
            return;
        }

        for (unsigned int i = 0; i < m_data->getNumPlanes(); ++i)
        {
            if (!m_data->isPlaneDataSet(i))
            {
                StringOutputStream message;
                message << "no dmabuf has been added for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE, message.release());
                return;
            }
        }

        m_data->setWidth(width);
        m_data->setHeight(height);
        m_data->setFormat(format);
        m_data->setFlags(flags);

        if (width < 1 || height < 1)
        {
            StringOutputStream message;
            message << "invalid width " << width << " or height " << height;
            m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS, message.release());
            return;
        }

        for (unsigned int i = 0; i < m_data->getNumPlanes(); ++i)
        {
            uint64_t const offset = static_cast<uint64_t>(m_data->getOffset(i));
            uint64_t const stride = static_cast<uint64_t>(m_data->getStride(i));

            // Validate internal consistency of user-supplied geometry info
            if (offset + stride > UINT32_MAX)
            {
                StringOutputStream message;
                message << "size overflow for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS, message.release());
                return;
            }

            if (i == 0 && offset + stride * m_data->getHeight() > UINT32_MAX)
            {
                StringOutputStream message;
                message << "size overflow for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS, message.release());
                return;
            }

            // Cross-check user-supplied geometry against size bounds reported by kernel
            off_t const size = ::lseek(m_data->getFd(i), 0, SEEK_END);

            if (-1 == size)
            {
                // Don't treat this as an error. Kernel doesn't always support seeking on a dmabuf.
                continue;
            }

            size_t const uSize = size;

            if (offset >= uSize)
            {
                StringOutputStream message;
                message << "invalid offset " << offset << " for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS, message.release());
                return;
            }

            if (offset + stride > uSize)
            {
                StringOutputStream message;
                message << "invalid stride " << offset << " for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS, message.release());
                return;
            }

            // On the first plane only, check width and height. Subsequent planes may use sub-sampling, so
            // we can't enforce this generically.
            if (0 == i && offset + stride * m_data->getHeight() > uSize)
            {
                StringOutputStream message;
                message << "invalid buffer stride or height for plane " << i;
                m_resource->postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS, message.release());
                return;
            }
        }

        IWaylandResource* bufferWaylandResource = client.resourceCreate(&wl_buffer_interface, 1 /* version */, bufferId);

        if (nullptr == bufferWaylandResource)
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinuxDmabufParams::createBuffer(): Could not create wayland resource");
            client.postNoMemory();
            return;
        }

        // Hand off ownership of the LinuxDmabufBufferData to the new wl_buffer
        wl_resource* bufferNativeWaylandResource = static_cast<wl_resource*>(bufferWaylandResource->getWaylandNativeResource());
        bufferWaylandResource->setImplementation(&LinuxDmabufBuffer::m_bufferInterface, m_data.release(), BufferDestroyCallback);

        // Announce the resulting buffer to the client
        if (0 == bufferId)
        {
            zwp_linux_buffer_params_v1_send_created(static_cast<wl_resource*>(m_resource->getWaylandNativeResource()), bufferNativeWaylandResource);
        }

        // Clean up
        bufferWaylandResource->disownWaylandResource();
        delete bufferWaylandResource;
    }

    void LinuxDmabufBuffer::DmabufBufferDestroyCallback(wl_client* client, wl_resource* bufferResource)
    {
        UNUSED(client)
        wl_resource_destroy(bufferResource);
    }

    LinuxDmabufBufferData* LinuxDmabufBuffer::fromWaylandBufferResource(WaylandBufferResource& resource)
    {
        struct wl_resource* nativeResource = static_cast<wl_resource*>(resource.getWaylandNativeResource());

        if (!wl_resource_instance_of(nativeResource, &wl_buffer_interface, &m_bufferInterface))
        {
            return nullptr;
        }

        LinuxDmabufBufferData* ret = static_cast<LinuxDmabufBufferData*>(wl_resource_get_user_data(nativeResource));
        assert(nullptr != ret);
        return ret;
    }
}
