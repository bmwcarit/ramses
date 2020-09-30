//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/IWaylandRegion.h"
#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "WaylandClientMock.h"
#include "NativeWaylandResourceMock.h"
#include "EmbeddedCompositor_WaylandMock.h"

#include "gtest/gtest.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandCompositorConnection : public Test
    {
    public:
        AWaylandCompositorConnection()
        {
        }

        ~AWaylandCompositorConnection()
        {
            if (nullptr != m_waylandCompositorConnection)
            {
                EXPECT_CALL(m_compositor, removeWaylandCompositorConnection(Ref(*m_waylandCompositorConnection)));
                if (nullptr != m_waylandCompositorConnectionResource)
                {
                    EXPECT_CALL(*m_waylandCompositorConnectionResource, setImplementation(_,m_waylandCompositorConnection,nullptr));
                }
                delete m_waylandCompositorConnection;
            }
        }

        void createWaylandCompositorConnection()
        {
            const uint32_t id               = 1;
            const uint32_t interfaceVersion = 4;

            m_waylandCompositorConnectionResource = new NativeWaylandResourceMock;

            InSequence s;
            EXPECT_CALL(m_client, resourceCreate(&wl_compositor_interface, interfaceVersion, id))
                .WillOnce(Return(m_waylandCompositorConnectionResource));

            EXPECT_CALL(*m_waylandCompositorConnectionResource, setImplementation(_,_,_));

            m_waylandCompositorConnection =
                new WaylandCompositorConnection(m_client, interfaceVersion, id, m_compositor);
        }

    protected:
        StrictMock<WaylandClientMock>              m_client;
        StrictMock<EmbeddedCompositor_WaylandMock> m_compositor;
        WaylandCompositorConnection* m_waylandCompositorConnection = nullptr;
        NativeWaylandResourceMock* m_waylandCompositorConnectionResource = nullptr;
    };

    TEST_F(AWaylandCompositorConnection, CanBeCreatedAndDestroyed)
    {
        createWaylandCompositorConnection();
    }

    TEST_F(AWaylandCompositorConnection, CantBeCreatedWhenResourceCreateFails)
    {
        const uint32_t id               = 1;
        const uint32_t interfaceVersion = 4;

        InSequence s;
        EXPECT_CALL(m_client, resourceCreate(&wl_compositor_interface, interfaceVersion, id))
            .WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());

        m_waylandCompositorConnection = new WaylandCompositorConnection(m_client, interfaceVersion, id, m_compositor);
    }



    TEST_F(AWaylandCompositorConnection, CanCreateSurface)
    {
        createWaylandCompositorConnection();

        const uint32_t id               = 1;
        const uint32_t interfaceVersion = 4;

        NativeWaylandResourceMock* surfaceResource = new NativeWaylandResourceMock;

        InSequence s;
        EXPECT_CALL(m_client, resourceCreate(&wl_surface_interface, interfaceVersion, id))
            .WillOnce(Return(surfaceResource));

        EXPECT_CALL(*surfaceResource, setImplementation(_,_,_));

        IWaylandSurface* createdWaylandSurface = nullptr;

        EXPECT_CALL(m_compositor, addWaylandSurface(_))
            .WillOnce(Invoke([&createdWaylandSurface](IWaylandSurface& waylandSurface)
            {
                createdWaylandSurface = &waylandSurface;
            }));

        m_waylandCompositorConnection->compositorCreateSurface(m_client, id);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*createdWaylandSurface)));
        EXPECT_CALL(*surfaceResource, setImplementation(_, createdWaylandSurface, nullptr));
        delete createdWaylandSurface;
    }

    TEST_F(AWaylandCompositorConnection, CanCreateRegion)
    {
        createWaylandCompositorConnection();

        const uint32_t id               = 1;
        const uint32_t interfaceVersion = 4;

        NativeWaylandResourceMock* regionResource = new NativeWaylandResourceMock;

        InSequence s;
        EXPECT_CALL(m_client, resourceCreate(&wl_region_interface, interfaceVersion, id))
            .WillOnce(Return(regionResource));

        EXPECT_CALL(*regionResource, setImplementation(_, _, _));

        IWaylandRegion* createdWaylandRegion = nullptr;

        EXPECT_CALL(m_compositor, addWaylandRegion(_))
            .WillOnce(Invoke([&createdWaylandRegion](IWaylandRegion& waylandRegion) {
                createdWaylandRegion = &waylandRegion;
            }));

        m_waylandCompositorConnection->compositorCreateRegion(m_client, id);

        EXPECT_CALL(m_compositor, removeWaylandRegion(Ref(*createdWaylandRegion)));
        EXPECT_CALL(*regionResource, setImplementation(_, createdWaylandRegion, nullptr));
        delete createdWaylandRegion;
    }
}
