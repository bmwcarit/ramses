//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Ramsh/RamshCommandArguments.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

#include <string>

namespace ramses::internal
{

    class SystemCompositorControllerScreenshot : public RamshCommandArgs<std::string, int32_t>
    {
    public:
        explicit SystemCompositorControllerScreenshot(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(std::string& fileName, int32_t& screenIviId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}
