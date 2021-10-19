//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDEVICEEXTENSION_H
#define RAMSES_IDEVICEEXTENSION_H

#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class IDeviceExtension
    {
    public:
        virtual ~IDeviceExtension() = default;

        virtual DeviceResourceHandle    createDmaRenderBuffer       (uint32_t width, uint32_t height, DmaBufferFourccFormat fourccFormat, DmaBufferUsageFlags usageFlags, DmaBufferModifiers modifiers) = 0;
        virtual int                     getDmaRenderBufferFD        (DeviceResourceHandle handle) = 0;
        virtual uint32_t                getDmaRenderBufferStride    (DeviceResourceHandle handle) = 0;
        virtual void                    destroyDmaRenderBuffer      (DeviceResourceHandle handle) = 0;
    };
}

#endif
