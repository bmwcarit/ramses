//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PRINTSTATISTICS_H
#define RAMSES_PRINTSTATISTICS_H

#include "Ramsh/RamshCommand.h"
#include "RendererLib/RendererStatistics.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class PrintStatistics : public RamshCommand
    {
    public:
        explicit PrintStatistics(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool executeInput(const RamshInput& input) override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
