//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/DeviceResourceMapper.h"
#include "internal/RendererLib/PlatformBase/GpuResource.h"

namespace ramses::internal
{
    DeviceResourceMapper::DeviceResourceMapper()
    {
        // preallocate memory for GPU resources to avoid resizing of growing pool for small sizes
        m_resources.preallocateSize(128u);
    }

    DeviceResourceMapper::~DeviceResourceMapper()
    {
        for (uint32_t i = 0; i < m_resources.getTotalCount(); ++i)
        {
            const DeviceResourceHandle handle(i);
            if (containsResource(handle))
            {
                deleteResource(handle);
            }
        }
    }

    uint32_t DeviceResourceMapper::getTotalGpuMemoryUsageInKB() const
    {
        return m_memoryUsage >> 10;
    }

    DeviceResourceHandle DeviceResourceMapper::registerResource(std::unique_ptr<const GPUResource> resource)
    {
        const DeviceResourceHandle handle = m_resources.allocate();
        m_memoryUsage += resource->getTotalSizeInBytes();
        *m_resources.getMemory(handle) = resource.release();

        return handle;
    }

    void DeviceResourceMapper::deleteResource(DeviceResourceHandle resourceHandle)
    {
        const GPUResource* const resource = *m_resources.getMemory(resourceHandle);
        assert(m_memoryUsage >= resource->getTotalSizeInBytes());
        m_memoryUsage -= resource->getTotalSizeInBytes();

        delete resource;
        m_resources.release(resourceHandle);
    }
}
