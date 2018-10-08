//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/PrintStatistics.h"
#include "Ramsh/RamshInput.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    PrintStatistics::PrintStatistics(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "print render statistics and reset measure interval";
        registerKeyword("printStatistics");
        registerKeyword(";");
    }

    Bool PrintStatistics::executeInput(const RamshInput& /*input*/)
    {
        m_rendererCommandBuffer.logStatistics();
        return true;
    }

}
