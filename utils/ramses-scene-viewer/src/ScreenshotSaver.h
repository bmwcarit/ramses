//  -------------------------------------------------------------------------
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_VIEWER_SCREENSHOTSAVER_H
#define RAMSES_SCENE_VIEWER_SCREENSHOTSAVER_H

#include <vector>
#include "Collections/String.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/RamsesRenderer.h"

namespace ramses
{
    class ScreenshotSaver final : public ramses::RendererEventHandlerEmpty
    {

    public:
        ScreenshotSaver(ramses::RamsesRenderer& renderer, ramses::displayId_t displayId, uint32_t width, uint32_t height, std::string filename);

        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
        virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;

    private:
        ramses::RamsesRenderer& m_renderer;
        ramses::displayId_t m_displayId;

        uint32_t m_width;
        uint32_t m_height;
        std::string m_filename;
    };
}

#endif
