//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RamshCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "internal/RendererLib/RendererCommandBuffer.h"


using namespace ramses::internal;

SetSkippingOfUnmodifiedBuffers::SetSkippingOfUnmodifiedBuffers(RendererCommandBuffer& rendererCommandBuffer)
: m_rendererCommandBuffer(rendererCommandBuffer)
{
    description = "set skipping of unmodified buffer";
    registerKeyword("skub");
    registerKeyword("skipUnmodifiedBuffers");
    getArgument<0>().setDescription("enable skipping (0: off, 1: enable)");
}

bool SetSkippingOfUnmodifiedBuffers::execute(uint32_t& enableSkipping) const
{
    m_rendererCommandBuffer.enqueueCommand(ramses::internal::RendererCommand::SetSkippingOfUnmodifiedBuffers{ enableSkipping > 0u });
    return true;
}
