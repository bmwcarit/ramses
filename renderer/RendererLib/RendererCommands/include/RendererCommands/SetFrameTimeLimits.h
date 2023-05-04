//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SETFRAMETIMELIMITS_H
#define RAMSES_SETFRAMETIMELIMITS_H

#include "Ramsh/RamshCommandArguments.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class SetFrameTimeLimits : public RamshCommandArgs<uint32_t, uint32_t, uint32_t>
    {
    public:
        explicit SetFrameTimeLimits(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(uint32_t& limitForDynamicResourcesUploadMicrosec, uint32_t& limitForStaticResourcesUploadMicrosec, uint32_t& limitForOffscreenBufferRenderMicrosec) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
