//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "internal/RendererLib/RamshCommands/PrintStatistics.h"
#include "internal/RendererLib/RendererCommandBuffer.h"

namespace ramses::internal
{
    PrintStatistics::PrintStatistics(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "print render statistics and reset measure interval";
        registerKeyword("printStatistics");
        registerKeyword(";");
    }

    bool PrintStatistics::executeInput(const std::vector<std::string>& /*input*/)
    {
        m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::LogStatistics{});
        return true;
    }

}
