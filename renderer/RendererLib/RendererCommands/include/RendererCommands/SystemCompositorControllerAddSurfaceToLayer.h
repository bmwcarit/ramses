//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERADDSURFACETOLAYER_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERADDSURFACETOLAYER_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerAddSurfaceToLayer : public RamshCommandArgs<UInt32, UInt32>
    {
    public:
        SystemCompositorControllerAddSurfaceToLayer(RendererCommandBuffer& rendererCommandBuffer);
        virtual Bool execute(UInt32& surfaceId, UInt32& layerId) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
