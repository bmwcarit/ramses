//  -------------------------------------------------------------------------
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cstring>
#include "Collections/String.h"
#include "ScreenshotSaver.h"
// TODO should not use ramses internals
#include "Utils/Image.h"

namespace ramses
{

        ScreenshotSaver::ScreenshotSaver(RamsesRenderer& renderer, displayId_t displayId, uint32_t width, uint32_t height, std::string filename)
            : m_renderer{renderer}
            , m_displayId{displayId}
            , m_width{width}
            , m_height{height}
            , m_filename(filename)
        {
        };

        void ScreenshotSaver::sceneShown(sceneId_t sceneId, ERendererEventResult result)
        {
            UNUSED(sceneId);
            if (result == ERendererEventResult_OK)
            {
                m_renderer.readPixels(m_displayId, 0, 0, m_width, m_height);
                m_renderer.flush();
            }
        }

        void ScreenshotSaver::framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, ERendererEventResult result)
        {
            UNUSED(displayId);
            if (result == ramses::ERendererEventResult_OK)
            {
                assert(pixelDataSize == m_width * m_height * 4);

                ramses_internal::Image image(m_width, m_height, pixelData, pixelData + pixelDataSize, true);
                image.saveToFilePNG(m_filename.c_str());
            }
        }

}
