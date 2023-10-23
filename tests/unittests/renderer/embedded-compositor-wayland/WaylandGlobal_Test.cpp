//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandGlobal.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    using namespace testing;

    class AWaylandGlobal : public Test
    {
    public:
        AWaylandGlobal()
        {
            m_display = wl_display_create();
        }

        ~AWaylandGlobal() override
        {
            if (m_display != nullptr)
            {
                wl_display_destroy(m_display);
            }
        }
    protected:

        wl_display* m_display = nullptr;
    };


    TEST_F(AWaylandGlobal, CanBeCreatedAndDestroyed)
    {
        wl_global* global = wl_global_create(m_display, &wl_compositor_interface, 1, this, nullptr);
        WaylandGlobal waylandGlobal(global);
    }
}
