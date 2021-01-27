//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "RendererCommands/SetClearColor.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    SetClearColor::SetClearColor(RendererCommandBuffer& rendererCommandBuffer)
        : m_rendererCommandBuffer(rendererCommandBuffer)
    {
        description = "set display clear color";
        registerKeyword("clc");
        getArgument<0>().setDescription("display id");
        getArgument<1>().setDescription("red channel value");
        getArgument<2>().setDescription("green channel value");
        getArgument<3>().setDescription("blue channel value");
        getArgument<4>().setDescription("alpha channel value");
    }

    Bool SetClearColor::execute(UInt32& displayId, Float& red, Float& green, Float& blue, Float& alpha) const
    {
        m_rendererCommandBuffer.enqueueCommand(ramses_internal::RendererCommand::SetClearColor{ DisplayHandle{displayId}, {}, Vector4{red, green, blue, alpha} });
        return true;
    }
}
