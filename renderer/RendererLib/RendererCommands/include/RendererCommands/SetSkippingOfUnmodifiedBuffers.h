//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SETSKIPPINGOFUNMODIFIEDBUFFERS_H
#define RAMSES_SETSKIPPINGOFUNMODIFIEDBUFFERS_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class SetSkippingOfUnmodifiedBuffers : public RamshCommandArgs<uint32_t>
    {
    public:
        explicit SetSkippingOfUnmodifiedBuffers(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint32_t& enableSkipping) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
