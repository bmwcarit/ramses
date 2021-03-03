//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCREENSHOT_H
#define RAMSES_SCREENSHOT_H

#include "Ramsh/RamshCommand.h"

namespace ramses_internal
{
    class RendererCommandBuffer;

    class Screenshot : public RamshCommand
    {
    public:
        explicit Screenshot(RendererCommandBuffer& rendererCommandBuffer);
        virtual bool executeInput(const std::vector<std::string>& input) override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
