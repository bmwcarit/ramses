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

    class AssignScene : public RamshCommand
    {
    public:
        explicit AssignScene(RendererCommandBuffer& rendererCommandBuffer);
        bool executeInput(const std::vector<std::string>& input) override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

