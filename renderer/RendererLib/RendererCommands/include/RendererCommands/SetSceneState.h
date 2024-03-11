//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SETSCENESTATE_H
#define RAMSES_SETSCENESTATE_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class SetSceneState : public RamshCommandArgs<uint64_t, uint32_t>
    {
    public:
        explicit SetSceneState(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(uint64_t& sceneId, uint32_t& sceneState) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
