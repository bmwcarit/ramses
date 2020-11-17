//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERMATERAMSHCOMMANDS_H
#define RAMSES_RENDERERMATERAMSHCOMMANDS_H

#include "Ramsh/RamshCommand.h"
#include "Ramsh/RamshCommandArguments.h"

namespace ramses
{
    class RendererMate;
}

namespace ramses_internal
{
    class RendererMateRamshCommand
    {
    public:
        explicit RendererMateRamshCommand(ramses::RendererMate& rendererMate);
        virtual ~RendererMateRamshCommand() = default;
    protected:
        ramses::RendererMate& m_rendererMate;
    };

    class ShowSceneOnDisplay final : public RamshCommand, public RendererMateRamshCommand
    {
    public:
        explicit ShowSceneOnDisplay(ramses::RendererMate& rendererMate);
        virtual bool executeInput(const RamshInput& input) override;
    };

    class HideScene final : public RamshCommandArgs < uint64_t >, public RendererMateRamshCommand
    {
    public:
        explicit HideScene(ramses::RendererMate& rendererMate);
        virtual bool execute(uint64_t& sceneId) const override;
    };

    class ReleaseScene final : public RamshCommandArgs < uint64_t >, public RendererMateRamshCommand
    {
    public:
        explicit ReleaseScene(ramses::RendererMate& rendererMate);
        virtual bool execute(uint64_t& sceneId) const override;
    };

    class LinkData final : public RamshCommandArgs < uint64_t, uint32_t, uint64_t, uint32_t >, public RendererMateRamshCommand
    {
    public:
        explicit LinkData(ramses::RendererMate& rendererMate);
        virtual bool execute(uint64_t& providerSceneId, uint32_t& providerId, uint64_t& consumerSceneId, uint32_t& consumerId) const override;
    };

    class ConfirmationEcho final : public RamshCommandArgs<String>, public RendererMateRamshCommand
    {
    public:
        explicit ConfirmationEcho(ramses::RendererMate& rendererMate);
        virtual bool execute(String& text) const override;
    };
}

#endif
