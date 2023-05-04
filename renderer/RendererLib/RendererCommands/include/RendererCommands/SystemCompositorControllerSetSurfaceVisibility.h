//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEVISIBILITY_H
#define RAMSES_SYSTEMCOMPOSITORCONTROLLERSETSURFACEVISIBILITY_H

#include "Ramsh/RamshCommandArguments.h"
#include "RendererLib/RendererCommandBuffer.h"

namespace ramses_internal
{

    class SystemCompositorControllerSetSurfaceVisibility : public RamshCommandArgs<Int32, Int32>
    {
    public:
        explicit SystemCompositorControllerSetSurfaceVisibility(RendererCommandBuffer& rendererCommandBuffer);
        bool execute(Int32& surfaceId, Int32& visibility) const override;

    private:
        RendererCommandBuffer& m_rendererCommandBuffer;
    };
}

#endif
