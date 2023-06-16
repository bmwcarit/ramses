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

    class SetClearColor : public RamshCommandArgs<uint32_t, float, float, float, float>
    {
    public:
        explicit SetClearColor(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint32_t& displayId, float& red, float& green, float& blue, float& alpha) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
