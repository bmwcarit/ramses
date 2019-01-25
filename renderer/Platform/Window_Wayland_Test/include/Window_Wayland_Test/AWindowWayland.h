//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_AWINDOWWAYLAND_H
#define RAMSES_AWINDOWWAYLAND_H

#include "WindowEventHandlerMock.h"
#include "TestWithWaylandEnvironment.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererTestUtils.h"
#include "DisplayConfigImpl.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "Utils/StringUtils.h"

namespace ramses_internal
{
    template <typename WINDOWTYPE>
    class AWindowWayland : public TestWithWaylandEnvironment
    {
    public:
        virtual void SetUp() override
        {
            createWaylandWindow();
        }

        virtual void TearDown() override
        {
            destroyWaylandWindow();
        }

    protected:
        void createWaylandWindow()
        {
            WaylandEnvironmentUtils::SetVariable(WaylandEnvironmentVariable::XDGRuntimeDir, m_initialValueOfXdgRuntimeDir);

            m_window = new WINDOWTYPE(m_config.impl.getInternalDisplayConfig(), m_eventHandlerMock, 0);
        }

        void destroyWaylandWindow()
        {
            delete m_window;
            m_window = nullptr;
        }

        ::testing::StrictMock<WindowEventHandlerMock> m_eventHandlerMock;
        ramses::DisplayConfig              m_config = RendererTestUtils::CreateTestDisplayConfig(0);
        WINDOWTYPE*                        m_window = nullptr;
    };
}

#endif
