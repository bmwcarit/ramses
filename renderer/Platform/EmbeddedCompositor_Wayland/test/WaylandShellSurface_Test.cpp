//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandShellSurface.h"
#include "gtest/gtest.h"

#include "WaylandClientMock.h"
#include "WaylandResourceMock.h"
#include "WaylandSurfaceMock.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandShellSurface : public Test
    {
    public:
        AWaylandShellSurface()
        {
        }

        ~AWaylandShellSurface()
        {
        }

        WaylandShellSurface* createWaylandShellSurface()
        {
            const uint32_t interfaceVersion = 1;
            const uint32_t id = 123;

            m_shellSurfaceResource = new StrictMock<WaylandResourceMock>;
            InSequence s;
            EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
            EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id))
                .WillOnce(Return(m_shellSurfaceResource));
            EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, _, _));
            EXPECT_CALL(m_surface, hasShellSurface()).WillOnce(Return(false));
            EXPECT_CALL(m_surface, setShellSurface(NotNull()));

            return new WaylandShellSurface(m_client, m_shellResource, id, m_surface);
        }

    protected:
        StrictMock<WaylandClientMock>   m_client;
        StrictMock<WaylandResourceMock> m_shellResource;
        StrictMock<WaylandSurfaceMock>  m_surface;
        StrictMock<WaylandResourceMock> m_seatResource;
        StrictMock<WaylandResourceMock> m_surfaceResource;
        StrictMock<WaylandResourceMock>* m_shellSurfaceResource = nullptr;
    };

    TEST_F(AWaylandShellSurface, CanBeCreatedAndDestroyed)
    {
        InSequence s;
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();

        EXPECT_CALL(m_surface, setShellSurface(nullptr));
        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CantBeCreatedWhenResourceCreateFails)
    {
        const uint32_t interfaceVersion = 1;
        const uint32_t id = 123;

        InSequence s;
        EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
        EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id)).WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        EXPECT_CALL(m_surface, hasShellSurface()).WillOnce(Return(false));
        EXPECT_CALL(m_surface, setShellSurface(NotNull()));
        EXPECT_CALL(m_surface, setShellSurface(nullptr));

        WaylandShellSurface shellSurface(m_client, m_shellResource, id, m_surface);
    }

    TEST_F(AWaylandShellSurface, CantBeCreatedWhenSurfaceAlreadyHasAShellSurface)
    {
        const uint32_t interfaceVersion = 1;
        const uint32_t id = 123;

        InSequence s;
        EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
        EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id)).WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        EXPECT_CALL(m_surface, hasShellSurface()).WillOnce(Return(true));
        EXPECT_CALL(m_shellResource, postError(WL_SHELL_ERROR_ROLE,_));

        WaylandShellSurface shellSurface(m_client, m_shellResource, id, m_surface);
    }

    TEST_F(AWaylandShellSurface, CanBeDestroyedWhenSurfaceWasAlreadyDeleted)
    {
        StrictMock<WaylandSurfaceMock>* surface = new StrictMock<WaylandSurfaceMock>;

        m_shellSurfaceResource = new StrictMock<WaylandResourceMock>;

        const uint32_t interfaceVersion = 1;
        const uint32_t id = 123;

        InSequence s;

        EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
        EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id))
            .WillOnce(Return(m_shellSurfaceResource));
        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, _, _));

        EXPECT_CALL(*surface, hasShellSurface()).WillOnce(Return(false));
        EXPECT_CALL(*surface, setShellSurface(NotNull()));

        WaylandShellSurface shellSurface(m_client, m_shellResource, id, *surface);

        delete surface;
        shellSurface.surfaceWasDeleted();

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_,&shellSurface, nullptr));
    }

    TEST_F(AWaylandShellSurface, CanSetAndGetShellSurfaceTitle)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->shellSurfaceSetTitle(m_client, "A Title");
        EXPECT_EQ(waylandShellSurface->getTitle(), "A Title");

        EXPECT_CALL(m_surface, setShellSurface(nullptr));
        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfacePong)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfacePong(m_client, 0);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceMove)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceMove(m_client, m_seatResource, 0);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceResize)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceResize(m_client, m_seatResource, 0, 0);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetToplevel)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetToplevel(m_client);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetTransient)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetTransient(m_client, m_surfaceResource, 0, 0, 0);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetFullscreen)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetFullscreen(m_client, 0, 60, nullptr);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetPopup)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetPopup(m_client, m_seatResource, 0, m_surfaceResource, 0, 0, 0);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetMaximized)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetMaximized(m_client, nullptr);

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }

    TEST_F(AWaylandShellSurface, CanShellSurfaceSetClass)
    {
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();
        waylandShellSurface->surfaceWasDeleted();
        waylandShellSurface->shellSurfaceSetClass(m_client, "ClassName");

        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, waylandShellSurface, nullptr));
        delete waylandShellSurface;
    }
}
