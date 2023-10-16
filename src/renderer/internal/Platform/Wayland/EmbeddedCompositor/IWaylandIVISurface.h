//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"

namespace ramses::internal
{
    class IWaylandBuffer;

    class IWaylandIVISurface
    {
    public:
        virtual ~IWaylandIVISurface() = default;
        virtual void resourceDestroyed() = 0;
        virtual void surfaceWasDeleted() = 0;
        virtual void bufferWasSetToSurface(IWaylandBuffer* buffer) = 0;
        [[nodiscard]] virtual WaylandIviSurfaceId getIviId() const = 0;
    };
}
