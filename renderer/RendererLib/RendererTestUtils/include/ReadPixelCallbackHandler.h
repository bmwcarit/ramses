//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_READPIXELCALLBACKHANDLER_H
#define RAMSES_READPIXELCALLBACKHANDLER_H

#include "ramses-renderer-api/IRendererEventHandler.h"

class ReadPixelCallbackHandler : public ramses::RendererEventHandlerEmpty
{
public:
    virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t, ramses::ERendererEventResult result) override
    {
        m_pixelData.clear();

        assert(result == ramses::ERendererEventResult_OK);
        UNUSED(result);
        m_pixelData.resize(pixelDataSize);
        ramses_internal::PlatformMemory::Copy(&m_pixelData[0], pixelData, pixelDataSize * sizeof(uint8_t));

        m_pixelDataRead = true;
    }

    ramses_internal::UInt8Vector m_pixelData;
    bool m_pixelDataRead = false;
};

#endif
