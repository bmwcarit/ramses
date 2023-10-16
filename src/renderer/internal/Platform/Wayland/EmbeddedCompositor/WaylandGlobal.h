//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandGlobal.h"
#include "wayland-server.h"

namespace ramses::internal
{
    class WaylandGlobal : public IWaylandGlobal
    {
    public:
        explicit WaylandGlobal(wl_global* global);
        ~WaylandGlobal() override;

    private:
        wl_global* m_global;
    };
}
