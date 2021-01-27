//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DEVICE_BASE_H
#define RAMSES_DEVICE_BASE_H

#include "RendererAPI/IDevice.h"
#include "RendererLimits.h"

namespace ramses_internal
{
    class IContext;
    class DeviceResourceMapper;

    class Device_Base : public IDevice
    {
    public:
        explicit Device_Base(IContext& context);

        // from IDevice
        virtual uint32_t getAndResetDrawCallCount() override;
        virtual void     drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual void     drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual uint32_t getGPUHandle(DeviceResourceHandle deviceHandle) const override;

        const RendererLimits& getRendererLimits() const;

    protected:
        const IContext&       m_context;
        DeviceResourceMapper& m_resourceMapper;

        RendererLimits m_limits;
        UInt32 m_drawCalls = 0u;
    };
}

#endif
