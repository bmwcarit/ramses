//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/PlatformBase/GpuResource.h"
#include "internal/Core/Utils/MemoryPool.h"

#include <cstdint>
#include <memory>
#include <cassert>

namespace ramses::internal
{
    class DeviceResourceMapper
    {
    public:
        DeviceResourceMapper();
        ~DeviceResourceMapper();

        DeviceResourceHandle    registerResource(std::unique_ptr<const GPUResource> resource);
        void                    deleteResource  (DeviceResourceHandle resourceHandle);
        [[nodiscard]] bool                    containsResource(DeviceResourceHandle resourceHandle) const;
        [[nodiscard]] const GPUResource&      getResource     (DeviceResourceHandle resourceHandle) const;

        template <typename TYPE>
        const TYPE&             getResourceAs   (DeviceResourceHandle resourceHandle) const;

        [[nodiscard]] uint32_t getTotalGpuMemoryUsageInKB() const;

    private:
        MemoryPool<const GPUResource*, DeviceResourceHandle> m_resources;
        uint32_t m_memoryUsage = 0u;
    };

    template <typename TYPE>
    const TYPE& DeviceResourceMapper::getResourceAs(DeviceResourceHandle resourceHandle) const
    {
        return static_cast<const TYPE&>(getResource(resourceHandle));
    }

    inline bool DeviceResourceMapper::containsResource(DeviceResourceHandle resourceHandle) const
    {
        return m_resources.isAllocated(resourceHandle);
    }

    inline const GPUResource& DeviceResourceMapper::getResource(DeviceResourceHandle resourceHandle) const
    {
        assert(containsResource(resourceHandle) && "resource handle is not mapped to a GPU resource!");
        return **m_resources.getMemory(resourceHandle);
    }
}
