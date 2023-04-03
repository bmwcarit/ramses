//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEDESTRECTANGLE_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEDESTRECTANGLE_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerSetSurfaceDestRectangle : public RamshCommandArgs<Int32, Int32, Int32, Int32, Int32>
    {
    public:
        explicit SystemCompositorControllerSetSurfaceDestRectangle(RendererCommandBuffer& rendererCommandBuffer);
        Bool execute(Int32& surfaceId, Int32& x, Int32& y, Int32& width, Int32& height) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
