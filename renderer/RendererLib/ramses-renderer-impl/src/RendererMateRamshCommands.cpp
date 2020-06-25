//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererMateRamshCommands.h"
#include "RendererMate.h"

namespace ramses_internal
{
    RendererMateRamshCommand::RendererMateRamshCommand(ramses::RendererMate& rendererMate)
        : m_rendererMate(rendererMate)
    {
    }

    ShowSceneOnDisplay::ShowSceneOnDisplay(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Subscribe to scene and map it on a display with translation offset (-sceneId # -displayId # -order # [-confirm <text>])";
        registerKeyword("showSceneOnDisplay");
    }

    uint32_t parseIntArg(const RamshInput& input, uint32_t idx)
    {
        const String argVal(input[idx]);
        return atoi(argVal.c_str());
    }

    float parseFloatArg(const RamshInput& input, uint32_t idx)
    {
        const String argVal(input[idx]);
        return static_cast<float>(atof(argVal.c_str()));
    }

    std::string parseStringArg(const RamshInput& input, uint32_t idx)
    {
        return input[idx].stdRef();
    }

    bool ShowSceneOnDisplay::executeInput(const RamshInput& input)
    {
        ramses::sceneId_t sceneId(0xffff);
        ramses::displayId_t displayId{ 0xffff };
        std::string confirmationText;
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
                displayId.getReference() = parseIntArg(input, ++argStrIdx);
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
            return false;

        m_rendererMate.setSceneMapping(sceneId, displayId);
        m_rendererMate.setSceneDisplayBufferAssignment(sceneId, {}, sceneRenderOrder);
        m_rendererMate.setSceneState(sceneId, ramses::RendererSceneState::Rendered, confirmationText);

        return true;
    }

    HideScene::HideScene(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Hide a mapped scene";
        registerKeyword("hideScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool HideScene::execute(uint64_t& sceneId) const
    {
        m_rendererMate.setSceneState(ramses::sceneId_t{ sceneId }, ramses::RendererSceneState::Ready);
        return true;
    }

    UnmapScene::UnmapScene(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Unmap a scene from a display";
        registerKeyword("unmapScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool UnmapScene::execute(uint64_t& sceneId) const
    {
        m_rendererMate.setSceneState(ramses::sceneId_t{ sceneId }, ramses::RendererSceneState::Available);
        return true;
    }

    UnsubscribeScene::UnsubscribeScene(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Unsubscribe renderer from a scene it is subscribed to";
        registerKeyword("unsubscribeScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool UnsubscribeScene::execute(uint64_t& sceneId) const
    {
        m_rendererMate.setSceneState(ramses::sceneId_t{ sceneId }, ramses::RendererSceneState::Unavailable);
        return true;
    }

    LinkData::LinkData(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Link scene data on renderer";
        registerKeyword("linkData");

        getArgument<0>()
            .registerKeyword("providerSceneId")
            .setDescription("Provider scene id");

        getArgument<1>()
            .registerKeyword("providerId")
            .setDescription("Provider data id");

        getArgument<2>()
            .registerKeyword("consumerSceneId")
            .setDescription("Consumer scene id");

        getArgument<3>()
            .registerKeyword("consumerId")
            .setDescription("Consumer data id");
    }

    bool LinkData::execute(uint64_t& providerSceneId, uint32_t& providerId, uint64_t& consumerSceneId, uint32_t& consumerId) const
    {
        m_rendererMate.linkData(ramses::sceneId_t(providerSceneId), ramses::dataProviderId_t{ providerId }, ramses::sceneId_t(consumerSceneId), ramses::dataConsumerId_t{ consumerId });
        return true;
    }

    ConfirmationEcho::ConfirmationEcho(ramses::RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "echos given text when command is executed (used for automated tests)";
        registerKeyword("confirm");

        getArgument<0>().setDescription("text");
    }

    bool ConfirmationEcho::execute(String& text) const
    {
        m_rendererMate.processConfirmationEchoCommand(text.stdRef());
        return true;
    }
}
