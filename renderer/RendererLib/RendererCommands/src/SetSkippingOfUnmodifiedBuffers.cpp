//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererCommands/SetSkippingOfUnmodifiedBuffers.h"
#include "RendererLib/RendererCommandBuffer.h"


using namespace ramses_internal;

SetSkippingOfUnmodifiedBuffers::SetSkippingOfUnmodifiedBuffers(RendererCommandBuffer& rendererCommandBuffer)
: m_rendererCommandBuffer(rendererCommandBuffer)
{
    description = "set skipping of unmodified buffer";
    registerKeyword("skub");
    registerKeyword("skipUnmodifiedBuffers");
    getArgument<0>().setDescription("enable skipping (0: off, 1: enable)");
}

Bool SetSkippingOfUnmodifiedBuffers::execute(UInt32& enableSkipping) const
{
    m_rendererCommandBuffer.setSkippingOfUnmodifiedBuffers( enableSkipping > 0u );
    return true;
}
