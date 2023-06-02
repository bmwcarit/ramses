//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/NativeWaylandResource.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "gtest/gtest.h"
#include "wayland-client.h"

namespace ramses_internal
{
    using namespace testing;

    class AWaylandResource : public Test
    {
    public:
        void SetUp() override
        {
            m_display = wl_display_create();
            ASSERT_TRUE(m_display != nullptr);
            const int serverFD = m_socket.createBoundFileDescriptor();
            m_client = wl_client_create(m_display, serverFD);
            ASSERT_TRUE(m_client != nullptr);
        }

        void TearDown() override
        {
            wl_client_destroy(m_client);
            wl_display_destroy(m_display);
        }

    protected:
        void resourceDestroyedListener(void* data)
        {
            UNUSED(data);
            m_resourceDestroyedListenerCalled++;
        }

        void resourceDestroyedCallback()
        {
            m_resourceDestroyedCallbackCalled++;
        }

        static void ResourceDestroyedListener(wl_listener* listener, void* data)
        {
            WARNINGS_PUSH
            WARNING_DISABLE_LINUX(-Winvalid-offsetof)
            WARNING_DISABLE_LINUX(-Wcast-align)
            WARNING_DISABLE_LINUX(-Wold-style-cast)

            AWaylandResource* waylandResource = wl_container_of(listener, waylandResource, m_destroyListener);
            waylandResource->resourceDestroyedListener(data);

            WARNINGS_POP
        }

        static void ResourceDestroyedCallback(wl_resource* clientResource)
        {
            UNUSED(clientResource);

            AWaylandResource* waylandResource = static_cast<AWaylandResource*>(wl_resource_get_user_data(clientResource));
            waylandResource->resourceDestroyedCallback();
        }

        wl_display* m_display = nullptr;
        UnixDomainSocket m_socket = UnixDomainSocket("testingSocket", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));
        wl_client* m_client = nullptr;
        wl_listener m_destroyListener;
        uint32_t m_resourceDestroyedListenerCalled = 0;
        uint32_t m_resourceDestroyedCallbackCalled = 0;

        const struct wl_compositor_interface m_compositorInterface = {nullptr, nullptr};
    };

    TEST_F(AWaylandResource, CanBeCreatedAndDestroyedWithTakingOwnership)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);
        {
            NativeWaylandResource waylandResource(resource);
            m_destroyListener.notify = ResourceDestroyedListener;
            waylandResource.addDestroyListener(&m_destroyListener);

            EXPECT_EQ(m_resourceDestroyedListenerCalled, 0u);
            waylandResource.destroy();
        }
        // waylandResource gots deleted here
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 1u);
    }

    TEST_F(AWaylandResource, CanBeCreatedAndDestroyedWithoutTakingOwnership)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);
        {
            NativeWaylandResource waylandResource(resource);
            m_destroyListener.notify = ResourceDestroyedListener;
            waylandResource.addDestroyListener(&m_destroyListener);
        }
        // waylandResource gots deleted here
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 0u);
        wl_resource_destroy(resource);
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 1u);
    }

    TEST_F(AWaylandResource, CanGetVersion)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 4, 0);
        NativeWaylandResource waylandResource(resource);

        EXPECT_EQ(waylandResource.getVersion(), 4);
        waylandResource.destroy();
    }

    TEST_F(AWaylandResource, CanGetUserData)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);
        wl_resource_set_user_data(resource, reinterpret_cast<void*>(123));

        NativeWaylandResource waylandResource(resource);
        EXPECT_EQ(waylandResource.getUserData(), reinterpret_cast<void*>(123));
        waylandResource.destroy();
    }

    TEST_F(AWaylandResource, CanSetImplementation)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);

        {
            NativeWaylandResource waylandResource(resource);

            waylandResource.setImplementation(&m_compositorInterface, this, ResourceDestroyedCallback);

            EXPECT_EQ(m_resourceDestroyedCallbackCalled, 0u);
            waylandResource.destroy();
        }
        // waylandResource gots deleted here
        EXPECT_EQ(m_resourceDestroyedCallbackCalled, 1u);
    }

    TEST_F(AWaylandResource, CanGetWaylandNativeResource)
    {
        wl_resource*    resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);
        NativeWaylandResource waylandResource(resource);

        EXPECT_EQ(waylandResource.getLowLevelHandle(), resource);
        waylandResource.destroy();
    }

    TEST_F(AWaylandResource, DoesNotDestroyWaylandNativeResourceWhenOwnershipLost)
    {
        wl_resource* resource = wl_resource_create(m_client, &wl_compositor_interface, 1, 0);
        {
            NativeWaylandResource waylandResource(resource);
            m_destroyListener.notify = ResourceDestroyedListener;
            waylandResource.addDestroyListener(&m_destroyListener);
        }
        // waylandResource gots deleted here
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 0u);
        wl_resource_destroy(resource);
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 1u);
    }
}
