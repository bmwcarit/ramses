//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererMateRamshCommands.h"
#include "impl/RendererMate.h"

namespace ramses::internal
{
    RendererMateRamshCommand::RendererMateRamshCommand(RendererMate& rendererMate)
        : m_rendererMate(rendererMate)
    {
    }

    ShowSceneOnDisplay::ShowSceneOnDisplay(RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Subscribe to scene and map it on a display with translation offset (-sceneId # -displayId # -order # [-confirm <text>])";
        registerKeyword("showSceneOnDisplay");
    }

    uint32_t parseIntArg(const std::vector<std::string>& input, uint32_t idx)
    {
        uint32_t value = 0u;
        ArgumentConverter<uint32_t>::tryConvert(input[idx], value);
        return value;
    }

    std::string parseStringArg(const std::vector<std::string>& input, uint32_t idx)
    {
        return input[idx];
    }

    bool ShowSceneOnDisplay::executeInput(const std::vector<std::string>& input)
    {
        sceneId_t sceneId(0xffff);
        displayId_t displayId{ 0xffff };
        std::string confirmationText;
        int sceneRenderOrder = 0;

        bool sceneDefined = false;
        bool displayDefined = false;

        const auto numArgStrings = static_cast<uint32_t>(input.size());
        for (uint32_t argStrIdx = 0u; argStrIdx < numArgStrings - 1; ++argStrIdx)
        {
            const std::string argStr(input[argStrIdx]);
            if (argStr == std::string("-sceneId"))
            {
                sceneId = sceneId_t(parseIntArg(input, ++argStrIdx));
                sceneDefined = true;
            }
            else if (argStr == std::string("-displayId"))
            {
                displayId.getReference() = parseIntArg(input, ++argStrIdx);
                displayDefined = true;
            }
            else if (argStr == std::string("-order"))
            {
                sceneRenderOrder = static_cast<int>(parseIntArg(input, ++argStrIdx));
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
        m_rendererMate.setSceneState(sceneId, RendererSceneState::Rendered, confirmationText);

        return true;
    }

    HideScene::HideScene(RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Hide a rendered scene";
        registerKeyword("hideScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool HideScene::execute(uint64_t& sceneId) const
    {
        m_rendererMate.setSceneState(sceneId_t{ sceneId }, RendererSceneState::Ready);
        return true;
    }

    ReleaseScene::ReleaseScene(RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "Release (Unmap and Unsubscribe) a scene from renderer";
        registerKeyword("releaseScene");

        getArgument<0>()
            .registerKeyword("sceneId")
            .setDescription("Scene id");
    }

    bool ReleaseScene::execute(uint64_t& sceneId) const
    {
        m_rendererMate.setSceneState(sceneId_t{ sceneId }, RendererSceneState::Available);
        return true;
    }

    LinkData::LinkData(RendererMate& rendererMate)
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
        m_rendererMate.linkData(sceneId_t(providerSceneId), dataProviderId_t{ providerId }, sceneId_t(consumerSceneId), dataConsumerId_t{ consumerId });
        return true;
    }

    ConfirmationEcho::ConfirmationEcho(RendererMate& rendererMate)
        : RendererMateRamshCommand(rendererMate)
    {
        description = "echos given text when command is executed (used for automated tests)";
        registerKeyword("confirm");

        getArgument<0>().setDescription("displayId");
        getArgument<1>().setDescription("text");
    }

    bool ConfirmationEcho::execute(uint32_t& displayId, std::string& text) const
    {
        m_rendererMate.processConfirmationEchoCommand(displayId_t{ displayId }, text);
        return true;
    }
}
