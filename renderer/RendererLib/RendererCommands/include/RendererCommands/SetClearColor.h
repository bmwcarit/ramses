//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SETCLEARCOLOR_H
#define RAMSES_SETCLEARCOLOR_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class SetClearColor : public RamshCommandArgs<UInt32, Float, Float, Float, Float>
    {
    public:
        explicit SetClearColor(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(UInt32& displayId, Float& red, Float& green, Float& blue, Float& alpha) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
