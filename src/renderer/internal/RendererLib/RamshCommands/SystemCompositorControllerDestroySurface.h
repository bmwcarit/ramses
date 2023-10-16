//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

namespace ramses::internal
{
    class SystemCompositorControllerDestroySurface : public RamshCommandArgs<uint32_t>
    {
    public:
        explicit SystemCompositorControllerDestroySurface(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint32_t& surfaceId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}
