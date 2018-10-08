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
    class Device_Base : public IDevice
    {
    public:
        Device_Base();

        // from IDevice
        virtual UInt32  getDrawCallCount() const override;
        virtual void    resetDrawCallCount() override;
        virtual void    drawIndexedTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;
        virtual void    drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount) override;

        const RendererLimits& getRendererLimits() const;

    protected:
        RendererLimits m_limits;
        UInt32         m_drawCalls;
    };
}

#endif
