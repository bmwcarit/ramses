//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "EmbeddedCompositor_Wayland/WaylandCallbackResource.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/ThreadBarrier.h"
#include "gtest/gtest.h"
#include "wayland-client.h"
#include <atomic>

namespace ramses_internal
{
    using namespace testing;

    class AWaylandClient : public Test
    {
    public:
        AWaylandClient()
        {
            const int serverFD = m_socket.createBoundFileDescriptor();
            m_waylandDisplay.init("", "", serverFD);
            m_display = m_waylandDisplay.get();
            assert(m_display != nullptr);
        }

        ~AWaylandClient()
        {
        }

    protected:

        static void CompositorBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
        {
            UNUSED(data)
            UNUSED(version)
            UNUSED(id)

            WaylandClient waylandClient(client);
            waylandClient.postNoMemory();
        }

        void resourceDestroyedListener()
        {
            m_resourceDestroyedListenerCalled++;
        }

        static void ResourceDestroyedListener(wl_listener* listener, void* data)
        {
            UNUSED(data)
            WARNINGS_PUSH
            WARNING_DISABLE_LINUX(-Winvalid-offsetof)
            WARNING_DISABLE_LINUX(-Wcast-align)

            AWaylandClient* waylandClient = static_cast<AWaylandClient*>(wl_container_of(listener, waylandClient, m_destroyListener));

            WARNINGS_POP

            waylandClient->resourceDestroyedListener();
        }

        UnixDomainSocket m_socket = UnixDomainSocket("testingSocket", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));
        WaylandDisplay m_waylandDisplay;
        wl_display* m_display = nullptr;
        uint32_t m_resourceDestroyedListenerCalled = 0;
        wl_listener m_destroyListener;
    };

    namespace
    {
        class ClientRunnable : public Runnable
        {
        public:
            explicit ClientRunnable(int fd)
                : m_fd(fd)
                , m_protocolErrorCode(0)
            {
            }

            virtual void run() override
            {
                m_startBarrier.wait();
                wl_display*  display  = wl_display_connect_to_fd(m_fd);
                wl_registry* registry = wl_display_get_registry(display);

                wl_registry_add_listener(registry, &m_registryListener, this);

                // Causes binding against compositor interface in RegistryHandleGlobalCallBack.
                // The server side answers with WL_DISPLAY_ERROR_NO_MEMORY.
                wl_display_roundtrip(display);

                // Protocol error appears with the next request, so do a roundtrip.
                // Could also be any other request.
                wl_display_roundtrip(display);

                // Store current protocol error value in member, which is checked by the test in the main thread.
                m_protocolErrorCode = wl_display_get_protocol_error(display, nullptr, nullptr);

                wl_registry_destroy(registry);
                wl_display_disconnect(display);
            }

            int getProtocolErrorCode() const
            {
                return m_protocolErrorCode;
            }

            void waitOnStart()
            {
                m_startBarrier.wait();
            }

        private:
            static void RegistryHandleGlobalCallBack(
                void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
            {
                UNUSED(data)
                UNUSED(version)

                if (strcmp(interface, "wl_compositor") == 0)
                {
                    wl_registry_bind(registry, name, &wl_compositor_interface, 1);
                }
            }

            static void RegistryHandleGlobalRemoveCallBack(void* data, struct wl_registry* registry, uint32_t name)
            {
                UNUSED(data)
                UNUSED(registry)
                UNUSED(name)
            }

            int                            m_fd;
            std::atomic<int>               m_protocolErrorCode;
            ramses_internal::ThreadBarrier m_startBarrier{2};

            const struct Registry_Listener : public wl_registry_listener
            {
                Registry_Listener()
                {
                    global        = RegistryHandleGlobalCallBack;
                    global_remove = RegistryHandleGlobalRemoveCallBack;
                }
            } m_registryListener;
        };
    }

    TEST_F(AWaylandClient, CanGetCredentials)
    {
        const int serverFD = m_socket.getBoundFileDescriptor();

        wl_client* client = wl_client_create(m_display, serverFD);
        WaylandClient waylandClient(client);

        pid_t processId;
        uid_t userId;
        gid_t groupId;

        waylandClient.getCredentials(processId, userId, groupId);

        EXPECT_EQ(processId, getpid());
        EXPECT_EQ(userId, getuid());
        EXPECT_EQ(groupId, getgid());

        wl_client_destroy(client);
    }

    TEST_F(AWaylandClient, CanPostNoMemory)
    {
        // Sending postNoMemory to a client is only allowed as an answer to a request, it can
        // not just be send without, this would lead to an crash in the wayland libraries.
        // So, this test sets up a client (ClientRunnable), running in a separate thread, which binds the compositor
        // interface. This leads to CompositorBindCallback being called for the main thread. There, postNoMemory is done
        // as answer to the client. ClientRunnable stores the current protocol error code (WL_DISPLAY_ERROR_NO_MEMORY)
        // in a member variable, which is checked at the end of the test.

        wl_global* compositorGlobal = wl_global_create(m_display, &wl_compositor_interface, 1, this, CompositorBindCallback);

        const int clientFD = m_socket.createConnectedFileDescriptor(false);

        ClientRunnable client(clientFD);
        PlatformThread clientThread("ClientApp");
        clientThread.start(client);
        client.waitOnStart();

        wl_event_loop* loop = wl_display_get_event_loop(m_display);

        while (clientThread.isRunning())
        {
            wl_event_loop_dispatch(loop, 1);
            wl_display_flush_clients(m_display);
        }

        wl_global_destroy(compositorGlobal);

        // Check that  WL_DISPLAY_ERROR_NO_MEMORY has arrived in ClientRunnable.
        EXPECT_EQ(WL_DISPLAY_ERROR_NO_MEMORY, client.getProtocolErrorCode());
    }

    TEST_F(AWaylandClient, CanCreateResourceAndSetsOwnership)
    {
        const int serverFD = m_socket.getBoundFileDescriptor();

        wl_client*    client = wl_client_create(m_display, serverFD);
        WaylandClient waylandClient(client);

        const int32_t version = 2;
        const uint32_t id = 0;
        IWaylandResource* resource = waylandClient.resourceCreate(&wl_compositor_interface, version, id);
        EXPECT_NE(nullptr, resource);
        wl_resource* waylandResource = static_cast<wl_resource*>(resource->getWaylandNativeResource());

        EXPECT_NE(nullptr, waylandResource);
        EXPECT_EQ(client, wl_resource_get_client(waylandResource));
        EXPECT_EQ(1, wl_resource_instance_of(waylandResource, &wl_compositor_interface, nullptr));
        EXPECT_EQ(version, resource->getVersion());

        m_destroyListener.notify = ResourceDestroyedListener;
        resource->addDestroyListener(&m_destroyListener);

        EXPECT_EQ(m_resourceDestroyedListenerCalled, 0u);
        delete resource;

        // This checks, that ownership flag of wl_resource was set to true in WaylandClient::resourceCreate.
        EXPECT_EQ(m_resourceDestroyedListenerCalled, 1u);

        wl_client_destroy(client);
    }

    TEST_F(AWaylandClient, CanCreateCallbackResource)
    {
        const int serverFD = m_socket.getBoundFileDescriptor();

        wl_client*    client = wl_client_create(m_display, serverFD);
        WaylandClient waylandClient(client);

        const int32_t  version  = 1;
        const uint32_t id       = 0;
        WaylandCallbackResource* resource = waylandClient.callbackResourceCreate(&wl_callback_interface, version, id);
        wl_resource* waylandResource = static_cast<wl_resource*>(resource->getWaylandNativeResource());

        EXPECT_NE(nullptr, waylandResource);
        EXPECT_EQ(client, wl_resource_get_client(waylandResource));
        EXPECT_EQ(1, wl_resource_instance_of(waylandResource, &wl_callback_interface, nullptr));
        EXPECT_EQ(version, resource->getVersion());

        delete resource;
        wl_client_destroy(client);
    }
}
