//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/Device_Base.h"
#include "internal/RendererLib/PlatformBase/DeviceResourceMapper.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"

namespace ramses::internal
{
    Device_Base::Device_Base(IContext& context)
        : m_context(context)
        , m_resourceMapper(context.getResources())
    {
    }

    const RendererLimits& Device_Base::getRendererLimits() const
    {
        return m_limits;
    }

    void Device_Base::drawIndexedTriangles(int32_t /*startOffset*/, int32_t /*elementCount*/, uint32_t /*instanceCount*/)
    {
        ++m_drawCalls;
    }

    void Device_Base::drawTriangles([[maybe_unused]] int32_t startOffset, [[maybe_unused]] int32_t elementCount, [[maybe_unused]] uint32_t instanceCount)
    {
        ++m_drawCalls;
    }

    uint32_t Device_Base::getGPUHandle(DeviceResourceHandle deviceHandle) const
    {
        return m_resourceMapper.getResource(deviceHandle).getGPUAddress();
    }

    uint32_t Device_Base::getAndResetDrawCallCount()
    {
        const auto dc = m_drawCalls;
        m_drawCalls = 0u;
        return dc;
    }
}
