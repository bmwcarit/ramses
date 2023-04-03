//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandShellSurface.h"

#include "WaylandClientMock.h"
#include "NativeWaylandResourceMock.h"
#include "WaylandSurfaceMock.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandShellSurface : public Test
    {
    public:
        AWaylandShellSurface()
        {
        }

        ~AWaylandShellSurface() override
        {
        }

        WaylandShellSurface* createWaylandShellSurface()
        {
            const uint32_t interfaceVersion = 1;
            const uint32_t id = 123;

            m_shellSurfaceResource = new StrictMock<NativeWaylandResourceMock>;
            InSequence s;
            EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
            EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id))
                .WillOnce(Return(m_shellSurfaceResource));
            EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, _, _));
            EXPECT_CALL(m_surface, hasShellSurface()).WillOnce(Return(false));
            EXPECT_CALL(m_surface, setShellSurface(NotNull()));

            return new WaylandShellSurface(m_client, m_shellResource, id, m_surface);
        }

        void destroyWaylandShellSurface(WaylandShellSurface& shellSurface, bool expectUnsetShellSurface = true)
        {
            if(expectUnsetShellSurface)
                EXPECT_CALL(m_surface, setShellSurface(nullptr));
            EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, &shellSurface, nullptr));
            EXPECT_CALL(*m_shellSurfaceResource, destroy());

            delete &shellSurface;
        }

    protected:
        StrictMock<WaylandClientMock>   m_client;
        StrictMock<NativeWaylandResourceMock> m_shellResource;
        StrictMock<WaylandSurfaceMock>  m_surface;
        StrictMock<NativeWaylandResourceMock> m_seatResource;
        StrictMock<NativeWaylandResourceMock> m_surfaceResource;
        StrictMock<NativeWaylandResourceMock>* m_shellSurfaceResource = nullptr;
    };

    TEST_F(AWaylandShellSurface, CanBeCreatedAndDestroyed)
    {
        InSequence s;
        WaylandShellSurface* waylandShellSurface = createWaylandShellSurface();

        destroyWaylandShellSurface(*waylandShellSurface);
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

        m_shellSurfaceResource = new StrictMock<NativeWaylandResourceMock>;

        const uint32_t interfaceVersion = 1;
        const uint32_t id = 123;

        InSequence s;

        EXPECT_CALL(m_shellResource, getVersion()).WillOnce(Return(interfaceVersion));
        EXPECT_CALL(m_client, resourceCreate(&wl_shell_surface_interface, interfaceVersion, id))
            .WillOnce(Return(m_shellSurfaceResource));
        EXPECT_CALL(*m_shellSurfaceResource, setImplementation(_, _, _));

        EXPECT_CALL(*surface, hasShellSurface()).WillOnce(Return(false));
        EXPECT_CALL(*surface, setShellSurface(NotNull()));

        WaylandShellSurface* shellSurface = new WaylandShellSurface(m_client, m_shellResource, id, *surface);

        delete surface;
        shellSurface->surfaceWasDeleted();

        destroyWaylandShellSurface(*shellSurface, false);
    }
}
