//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERSETLAYERVISIBILITY_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERSETLAYERVISIBILITY_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerSetLayerVisibility : public RamshCommandArgs<Int32, Int32>
    {
    public:
        SystemCompositorControllerSetLayerVisibility(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(Int32& layerId, Int32& visibility) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
