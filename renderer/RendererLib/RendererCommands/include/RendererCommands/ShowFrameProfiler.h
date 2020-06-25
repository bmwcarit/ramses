//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SHOWFRAMEPROFILER_H
#define RAMSES_SHOWFRAMEPROFILER_H

#include "Ramsh/RamshCommand.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{
    class Renderer;

    class ShowFrameProfiler : public RamshCommand
    {
    public:
        explicit ShowFrameProfiler(RendererCommandBuffer& commandBuffer);
        virtual Bool executeInput(const RamshInput& input) override;

    private:
        RendererCommandBuffer& m_commandBuffer;
        void printHelpForRegionFiltering();

    };
}

#endif
