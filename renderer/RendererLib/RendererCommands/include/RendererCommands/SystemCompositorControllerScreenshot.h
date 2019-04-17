//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERSCREENSHOT_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERSCREENSHOT_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerScreenshot : public RamshCommandArgs<String, int32_t>
    {
    public:
        SystemCompositorControllerScreenshot(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(String& fileName, int32_t& screenIviId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
