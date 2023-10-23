//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/IRendererEventHandler.h"

namespace ramses::internal
{
    class ReadPixelCallbackHandler : public RendererEventHandlerEmpty
    {
    public:
        void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t /*displayId*/, displayBufferId_t /*displayBuffer*/, [[maybe_unused]] ERendererEventResult result) override
        {
            m_pixelData.clear();

            assert(result == ERendererEventResult::Ok);
            m_pixelData.resize(pixelDataSize);
            PlatformMemory::Copy(&m_pixelData[0], pixelData, pixelDataSize * sizeof(uint8_t));

            m_pixelDataRead = true;
        }

        std::vector<uint8_t> m_pixelData;
        bool m_pixelDataRead = false;
    };
}
