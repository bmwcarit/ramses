//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEOPACITY_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEOPACITY_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerSetSurfaceOpacity : public RamshCommandArgs<Int32, float>
    {
    public:
        explicit SystemCompositorControllerSetSurfaceOpacity(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(Int32& surfaceId, float& opacity) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
