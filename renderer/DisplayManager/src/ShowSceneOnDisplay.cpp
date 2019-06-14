//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManager/ShowSceneOnDisplay.h"

namespace ramses_display_manager
{
    ShowSceneOnDisplay::ShowSceneOnDisplay(IDisplayManager& displayManager)
        : m_displayManager(displayManager)
    {
        description = "Subscribe to scene and map it on a display with translation offset (-sceneId # -displayId # -order # [-confirm <text>])";
        registerKeyword("showSceneOnDisplay");
    }

    uint32_t parseIntArg(const ramses_internal::RamshInput& input, uint32_t idx)
    {
        const ramses_internal::String argVal(input[idx]);
        return atoi(argVal.c_str());
    }

    float parseFloatArg(const ramses_internal::RamshInput& input, uint32_t idx)
    {
        const ramses_internal::String argVal(input[idx]);
        return static_cast<float>(atof(argVal.c_str()));
    }

    std::string parseStringArg(const ramses_internal::RamshInput& input, uint32_t idx)
    {
        return input[idx].stdRef();
    }

    bool ShowSceneOnDisplay::executeInput(const ramses_internal::RamshInput& input)
    {
        ramses::sceneId_t sceneId = 0xffff;
        ramses::displayId_t displayId = 0xffff;
        std::string confirmationText = "";
        int sceneRenderOrder = 0;

        bool sceneDefined = false;
        bool displayDefined = false;

        const uint32_t numArgStrings = static_cast<uint32_t>(input.size());
        for (uint32_t argStrIdx = 0u; argStrIdx < numArgStrings - 1; ++argStrIdx)
        {
            const std::string argStr(input[argStrIdx].stdRef());
            if (argStr == std::string("-sceneId"))
            {
                sceneId = ramses::sceneId_t(parseIntArg(input, ++argStrIdx));
                sceneDefined = true;
            }
            else if (argStr == std::string("-displayId"))
            {
                displayId = parseIntArg(input, ++argStrIdx);
                displayDefined = true;
            }
            else if (argStr == std::string("-order"))
            {
                sceneRenderOrder = parseIntArg(input, ++argStrIdx);
            }
            else if (argStr == std::string("-confirm"))
            {
                confirmationText = parseStringArg(input, ++argStrIdx);
            }
        }

        if (!sceneDefined || !displayDefined)
        {
            return false;
        }

        m_displayManager.setSceneMapping(sceneId, displayId, sceneRenderOrder);
        m_displayManager.setSceneState(sceneId, ramses_display_manager::SceneState::Rendered, confirmationText.c_str());

        return true;
    }
}
