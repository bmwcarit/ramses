//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandGlobal.h"
#include "assert.h"

namespace ramses_internal
{
    WaylandGlobal::WaylandGlobal(wl_global* global)
        : m_global(global)
    {
        assert(m_global != nullptr);
    }

    WaylandGlobal::~WaylandGlobal()
    {
        wl_global_destroy(m_global);
    }
}
