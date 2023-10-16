//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "RendererLimits.h"

namespace ramses::internal
{
    class IContext;
    class DeviceResourceMapper;

    class Device_Base : public IDevice
    {
    public:
        explicit Device_Base(IContext& context);

        // from IDevice
        uint32_t getAndResetDrawCallCount() override;
        void     drawIndexedTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount) override;
        void     drawTriangles(int32_t startOffset, int32_t elementCount, uint32_t instanceCount) override;
        [[nodiscard]] uint32_t getGPUHandle(DeviceResourceHandle deviceHandle) const override;

        [[nodiscard]] const RendererLimits& getRendererLimits() const;

    protected:
        const IContext&       m_context;
        DeviceResourceMapper& m_resourceMapper;

        RendererLimits m_limits;
        uint32_t m_drawCalls = 0u;
    };
}
