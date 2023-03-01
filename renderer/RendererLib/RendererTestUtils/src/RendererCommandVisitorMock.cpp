//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommandVisitorMock.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    RendererCommandVisitorMock::RendererCommandVisitorMock() = default;
    RendererCommandVisitorMock::~RendererCommandVisitorMock() = default;

    void RendererCommandVisitorMock::visit(const RendererCommands& cmds)
    {
        for (const auto& c : cmds)
            std::visit(*this, c);
    }

    void RendererCommandVisitorMock::visit(RendererCommandBuffer& buffer)
    {
        RendererCommands cmds;
        buffer.swapCommands(cmds);
        visit(cmds);
    }
}
