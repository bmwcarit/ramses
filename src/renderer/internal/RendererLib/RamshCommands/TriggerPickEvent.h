//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
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

    class TriggerPickEvent : public RamshCommandArgs<uint64_t, float, float>
    {
    public:
        explicit TriggerPickEvent(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint64_t& sceneId, float& pickCoordX, float& pickCoordY) const override;
    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}
