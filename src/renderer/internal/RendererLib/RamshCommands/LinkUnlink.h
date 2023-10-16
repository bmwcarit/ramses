//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"

namespace ramses::internal
{
    class RendererCommandBuffer;

    class LinkBuffer : public RamshCommandArgs<uint64_t, uint32_t, uint32_t>
    {
    public:
        explicit LinkBuffer(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint64_t& sceneId, uint32_t& dataSlot, uint32_t& bufferId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };

    class UnlinkBuffer : public RamshCommandArgs<uint64_t, uint32_t>
    {
    public:
        explicit UnlinkBuffer(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint64_t& sceneId, uint32_t& dataSlot) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

