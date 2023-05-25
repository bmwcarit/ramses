//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRIGGERPICKEVENT_H
#define RAMSES_TRIGGERPICKEVENT_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class TriggerPickEvent : public RamshCommandArgs<UInt64, float, float>
    {
    public:
        explicit TriggerPickEvent(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(UInt64& sceneId, float& pickCoordX, float& pickCoordY) const override;
    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
