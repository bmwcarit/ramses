//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandCallbackResource.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/ThreadBarrier.h"
#include "gtest/gtest.h"
#include "wayland-client.h"
#include "wayland-client-protocol.h"
#include "wayland-server.h"
#include "Utils/LogMacros.h"
#include <atomic>

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        class ClientRunnable : public Runnable
        {
        public:
            ClientRunnable()
                : m_callbackReceived(false)
            {
            }

            void init(int fd)
            {
                m_fd = fd;
            }

            virtual void run() override
            {
                m_startBarrier.wait();
                wl_display*  display  = wl_display_connect_to_fd(m_fd);
                wl_registry* registry = wl_display_get_registry(display);

                wl_registry_add_listener(registry, &m_registryListener, this);


                // Causes binding against display interface in RegistryHandleGlobalCallBack and creates sync callback there.
                wl_display_roundtrip(display);

                // Causes sync callback beeing received
                wl_display_roundtrip(display);

                wl_registry_destroy(registry);
                wl_display_disconnect(display);
            }

            bool getCallbackReceived() const
            {
                return m_callbackReceived;
            }

            void waitOnStart()
            {
                m_startBarrier.wait();
            }

        private:
            static void RegistryHandleGlobalCallBack(
                void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
            {
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);
                if (strcmp(interface, "wl_display") == 0)
                {
                    wl_display* display =
                        static_cast<wl_display*>(wl_registry_bind(registry, name, &wl_display_interface, version));

                    if (nullptr != display)
                    {
                        wl_callback* callback = wl_display_sync(display);
                        wl_callback_add_listener(callback, &clientRunnable->m_syncCallbackListener, data);
                    }
                }
            }

            static void RegistryHandleGlobalRemoveCallBack(void* data, wl_registry* wl_registry, uint32_t name)
            {
                UNUSED(data)
                UNUSED(wl_registry)
                UNUSED(name)
            }

            static void SyncCallbackDone(void *data, wl_callback* callback, uint32_t callbackData)
            {
                UNUSED(callback)
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);
                EXPECT_EQ(33u, callbackData);
                clientRunnable->m_callbackReceived = true;
            }

            int                            m_fd = 0;
            std::atomic<bool>              m_callbackReceived;
            ramses_internal::ThreadBarrier m_startBarrier{2};

            const struct Registry_Listener : public wl_registry_listener
            {
                Registry_Listener()
                {
                    global        = RegistryHandleGlobalCallBack;
                    global_remove = RegistryHandleGlobalRemoveCallBack;
                }
            } m_registryListener;

            const struct SyncCallback_Listener : public wl_callback_listener
            {
                SyncCallback_Listener()
                {
                    done = SyncCallbackDone;
                }
            } m_syncCallbackListener;
        };
    }

    class AWaylandCallbackResource : public Test
    {
    public:
        virtual void SetUp() override
        {
            const int serverFD = m_socket.createBoundFileDescriptor();
            m_waylandDisplay.init("", "", 0, serverFD);
            m_display = m_waylandDisplay.get();
            ASSERT_TRUE(m_display != nullptr);
        }

        virtual void TearDown() override
        {
            if (m_client != nullptr)
                wl_client_destroy(m_client);
        }

        void startClientAndDispatchEventsUntilClientHasFinished()
        {
            wl_global* outputGlobal = wl_global_create(m_display, &wl_display_interface, 1, this, DisplayBindCallback);

            const int clientFD = m_socket.createConnectedFileDescriptor(false);

            m_clientRunnable.init(clientFD);
            PlatformThread clientThread("ClientApp");
            clientThread.start(m_clientRunnable);
            m_clientRunnable.waitOnStart();

            wl_event_loop* loop = wl_display_get_event_loop(m_display);

            while (clientThread.isRunning())
            {
                wl_event_loop_dispatch(loop, 1);
                wl_display_flush_clients(m_display);
            }

            wl_global_destroy(outputGlobal);
        }

    protected:
        static void DisplayBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
        {
            AWaylandCallbackResource* awaylandCallbackResource = static_cast<AWaylandCallbackResource*>(data);
            wl_resource*          resource = wl_resource_create(client, &wl_display_interface, version, id);
            wl_resource_set_implementation(resource, &awaylandCallbackResource->m_displayInterface, data, nullptr);
        }

        static void DisplaySync(struct wl_client* client, wl_resource* resource, uint32_t id)
        {
            UNUSED(client)
            UNUSED(resource)

            wl_resource* callbackResource = wl_resource_create(client, &wl_callback_interface, 1, id);
            WaylandCallbackResource waylandCallbackResource(callbackResource);
            waylandCallbackResource.callbackSendDone(33);
            waylandCallbackResource.destroy();
        }

        static void GetRegistry(struct wl_client* client, struct wl_resource* resource, uint32_t registry)
        {
            UNUSED(client)
            UNUSED(registry)
            UNUSED(resource)
        }

        const struct Display_Interface : public wl_display_interface
        {
            Display_Interface()
            {
                sync         = DisplaySync;
                get_registry = GetRegistry;
            }
        } m_displayInterface;

        WaylandDisplay         m_waylandDisplay;
        wl_display*            m_display = nullptr;
        UnixDomainSocket       m_socket  = UnixDomainSocket("testingSocket", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));
        wl_client*             m_client  = nullptr;
        ClientRunnable         m_clientRunnable;

        enum ESendEventType
        {
            ESendEventType_SendGeometry = 0,
            ESendEventType_SendMode,
            ESendEventType_SendScale,
            ESendEventType_SendDone
        };

        ESendEventType m_sendEventType = ESendEventType_SendGeometry;
    };

    TEST_F(AWaylandCallbackResource, CanBeCreatedAndDestroyed)
    {
        const int serverFD = m_socket.getBoundFileDescriptor();
        m_client = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);
        wl_resource* resource = wl_resource_create(m_client, &wl_callback_interface, 1, 0);
        WaylandCallbackResource waylandCallbackResource(resource);
        waylandCallbackResource.destroy();
    }

    TEST_F(AWaylandCallbackResource, CanSendDone)
    {
        startClientAndDispatchEventsUntilClientHasFinished();

        EXPECT_TRUE(m_clientRunnable.getCallbackReceived());
    }
}
