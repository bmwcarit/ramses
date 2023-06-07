//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AppearanceEnumsImpl.h"

namespace ramses
{
    const char* toString(EBlendOperation blendOperation)
    {
        return EnumToString(blendOperation);
    }

    const char* toString(EBlendFactor blendFactor)
    {
        return EnumToString(blendFactor);
    }

    const char* toString(ECullMode cullMode)
    {
        return EnumToString(cullMode);
    }

    const char* toString(EDepthWrite depthWrite)
    {
        return EnumToString(depthWrite);
    }

    const char* toString(EScissorTest scissorTest)
    {
        return EnumToString(scissorTest);
    }

    const char* toString(EDepthFunc depthFunc)
    {
        return EnumToString(depthFunc);
    }

    const char* toString(EStencilFunc stencilFunc)
    {
        return EnumToString(stencilFunc);
    }

    const char* toString(EStencilOperation stencilOp)
    {
        return EnumToString(stencilOp);
    }

    const char* toString(EDrawMode drawMode)
    {
        return EnumToString(drawMode);
    }
}
