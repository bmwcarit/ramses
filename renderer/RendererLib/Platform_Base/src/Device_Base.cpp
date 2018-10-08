//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/Device_Base.h"

namespace ramses_internal
{
    Device_Base::Device_Base()
        : m_drawCalls(0)
    {
    }

    const RendererLimits& Device_Base::getRendererLimits() const
    {
        return m_limits;
    }

    void Device_Base::drawIndexedTriangles(Int32, Int32, UInt32)
    {
        ++m_drawCalls;
    }

    void Device_Base::drawTriangles(Int32 startOffset, Int32 elementCount, UInt32 instanceCount)
    {
        UNUSED(startOffset)
        UNUSED(elementCount)
        UNUSED(instanceCount)
        ++m_drawCalls;
    }

    UInt32 Device_Base::getDrawCallCount() const
    {
        return m_drawCalls;
    }

    void Device_Base::resetDrawCallCount()
    {
        m_drawCalls = 0;
    }
}
