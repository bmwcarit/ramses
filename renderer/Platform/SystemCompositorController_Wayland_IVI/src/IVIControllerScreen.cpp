//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SystemCompositorController_Wayland_IVI/IVIControllerScreen.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"

PUSH_DISABLE_C_STYLE_CAST_WARNING
#include "ivi-controller-client-protocol.h"
POP_DISABLE_C_STYLE_CAST_WARNING

namespace ramses_internal
{
    IVIControllerScreen::IVIControllerScreen(ivi_controller_screen& controllerScreen, uint32_t screenId)
        : m_controllerScreen(controllerScreen)
        , m_screenId(screenId)
    {
    }

    IVIControllerScreen::~IVIControllerScreen()
    {
        LOG_INFO(CONTEXT_RENDERER, "IVIControllerSurface::~IVIControllerSurface screen-id: " << m_screenId);
        ivi_controller_screen_destroy(&m_controllerScreen);
    }

    uint32_t IVIControllerScreen::getScreenId() const
    {
        return m_screenId;
    }

    void IVIControllerScreen::takeScreenshot(const std::string& fileName)
    {
        ivi_controller_screen_screenshot(&m_controllerScreen, fileName.c_str());
    }
}
