//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommand.h"
#include "internal/Ramsh/RamshCommandArguments.h"

#include <string>

namespace ramses::internal
{
    class RendererMate;

    class RendererMateRamshCommand
    {
    public:
        explicit RendererMateRamshCommand(RendererMate& rendererMate);
        virtual ~RendererMateRamshCommand() = default;
    protected:
        RendererMate& m_rendererMate;
    };

    class ShowSceneOnDisplay final : public RamshCommand, public RendererMateRamshCommand
    {
    public:
        explicit ShowSceneOnDisplay(RendererMate& rendererMate);
        bool executeInput(const std::vector<std::string>& input) override;
    };

    class HideScene final : public RamshCommandArgs < uint64_t >, public RendererMateRamshCommand
    {
    public:
        explicit HideScene(RendererMate& rendererMate);
        bool execute(uint64_t& sceneId) const override;
    };

    class ReleaseScene final : public RamshCommandArgs < uint64_t >, public RendererMateRamshCommand
    {
    public:
        explicit ReleaseScene(RendererMate& rendererMate);
        bool execute(uint64_t& sceneId) const override;
    };

    class LinkData final : public RamshCommandArgs < uint64_t, uint32_t, uint64_t, uint32_t >, public RendererMateRamshCommand
    {
    public:
        explicit LinkData(RendererMate& rendererMate);
        bool execute(uint64_t& providerSceneId, uint32_t& providerId, uint64_t& consumerSceneId, uint32_t& consumerId) const override;
    };

    class ConfirmationEcho final : public RamshCommandArgs<uint32_t, std::string>, public RendererMateRamshCommand
    {
    public:
        explicit ConfirmationEcho(RendererMate& rendererMate);
        bool execute(uint32_t& displayId, std::string& text) const override;
    };
}
