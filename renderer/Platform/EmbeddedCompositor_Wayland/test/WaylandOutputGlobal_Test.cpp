//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputGlobal.h"
#include "WaylandDisplayMock.h"
#include "WaylandGlobalMock.h"
#include "WaylandClientMock.h"
#include "WaylandOutputResourceMock.h"

#include "PlatformAbstraction/PlatformMath.h"

#include "gtest/gtest.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandOutputGlobal : public Test
    {
    public:
        void createAndInitializeWaylandOutputGlobal()
        {
            m_waylandOutputGlobal = new WaylandOutputGlobal;

            m_waylandGlobalMock = new StrictMock<WaylandGlobalMock>;

            const int supportedOutputInterfaceVersion = min(3, wl_output_interface.version);
            EXPECT_CALL(m_serverDisplayMock, createGlobal(&wl_output_interface, supportedOutputInterfaceVersion,_,_))
                            .WillOnce(Return(m_waylandGlobalMock));

            EXPECT_TRUE(m_waylandOutputGlobal->init(m_serverDisplayMock));
        }

        void deleteWaylandOutputGlobal()
        {
            m_waylandOutputGlobal->destroy();
            delete m_waylandOutputGlobal;
        }

    protected:
        StrictMock<WaylandDisplayMock> m_serverDisplayMock;
        WaylandOutputGlobal* m_waylandOutputGlobal = nullptr;
        StrictMock<WaylandGlobalMock>* m_waylandGlobalMock = nullptr;
        StrictMock<WaylandClientMock> m_client;
    };

    TEST_F(AWaylandOutputGlobal, CanBeInitializedAndDestroyed)
    {
        createAndInitializeWaylandOutputGlobal();
        deleteWaylandOutputGlobal();
    }

    TEST_F(AWaylandOutputGlobal, ReturnsCorrectResolutionAndRefreshRate)
    {
        createAndInitializeWaylandOutputGlobal();
        int32_t width = 0;
        int32_t height = 0;
        m_waylandOutputGlobal->getResolution(width, height);


        // targets/platforms are delivering different wayland output resolutions,
        // so check just for any of these resolutions.
        EXPECT_TRUE((width == 1920 && height == 720) || (width == 1024 && height == 640) ||
                    (width == 1440 && height == 540));

        // Refresh rate values vary slightly around 60000 on some platforms
        int32_t refreshRate = m_waylandOutputGlobal->getRefreshRate();
        EXPECT_LE(60000, refreshRate);
        EXPECT_GE(60100, refreshRate);

        deleteWaylandOutputGlobal();
    }

    TEST_F(AWaylandOutputGlobal, InitFailsWhenOutputGlobalCanNotBeCreated)
    {
        m_waylandOutputGlobal = new WaylandOutputGlobal;

        const int supportedOutputInterfaceVersion = min(3, wl_output_interface.version);
        EXPECT_CALL(m_serverDisplayMock, createGlobal(&wl_output_interface, supportedOutputInterfaceVersion, _, _))
            .WillOnce(Return(nullptr));

        EXPECT_FALSE(m_waylandOutputGlobal->init(m_serverDisplayMock));

        deleteWaylandOutputGlobal();
    }
}
