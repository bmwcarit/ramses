//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputResource.h"
#include "UnixUtilities/UnixDomainSocketHelper.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "WaylandUtilities/WaylandUtilities.h"
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
                : m_outputGeometryReceived(false)
                , m_outputModeReceived(false)
                , m_outputScaleReceived(false)
                , m_outputDoneReceived(false)
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

                // Causes binding against output interface in RegistryHandleGlobalCallBack
                wl_display_roundtrip(display);

                // Causes output event beeing received
                wl_display_roundtrip(display);

                wl_registry_destroy(registry);
                wl_display_disconnect(display);
            }

            bool getOutputGeometryReceived() const
            {
                return m_outputGeometryReceived;
            }

            bool getOutputModeReceived() const
            {
                return m_outputModeReceived;
            }

            bool getOutputScaleReceived() const
            {
                return m_outputScaleReceived;
            }

            bool getOutputDoneReceived() const
            {
                return m_outputDoneReceived;
            }

            void waitOnStart()
            {
                m_startBarrier.wait();
            }

        private:
            static void RegistryHandleGlobalCallBack(
                void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
            {
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);
                if (strcmp(interface, "wl_output") == 0)
                {
                    wl_output* output =
                        static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, version));

                    if (nullptr != output)
                    {
                        wl_output_add_listener(output, &clientRunnable->m_outputListener, clientRunnable);
                    }
                }
            }

            static void RegistryHandleGlobalRemoveCallBack(void* data, wl_registry* wl_registry, uint32_t name)
            {
                UNUSED(data)
                UNUSED(wl_registry)
                UNUSED(name)
            }

            static void OutputHandleGeometry(void*       data,
                                             wl_output*  output,
                                             int32_t     x,
                                             int32_t     y,
                                             int32_t     physical_width,
                                             int32_t     physical_height,
                                             int32_t     subpixel,
                                             const char* make,
                                             const char* model,
                                             int32_t     transform)
            {
                UNUSED(output)
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);

                EXPECT_EQ(1, x);
                EXPECT_EQ(2, y);
                EXPECT_EQ(300, physical_width);
                EXPECT_EQ(400, physical_height);
                EXPECT_EQ(WL_OUTPUT_SUBPIXEL_VERTICAL_RGB, subpixel);
                EXPECT_STREQ("make", make);
                EXPECT_STREQ("model", model);
                EXPECT_EQ(WL_OUTPUT_TRANSFORM_90, transform);
                clientRunnable->m_outputGeometryReceived = true;
            }

            static void OutputHandleMode(
                void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
            {
                UNUSED(output)
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);

                EXPECT_EQ(static_cast<uint32_t>(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED), flags);
                EXPECT_EQ(300, width);
                EXPECT_EQ(400, height);
                EXPECT_EQ(60, refresh);
                clientRunnable->m_outputModeReceived = true;
            }

            static void OutputHandleScale(void* data, wl_output* output, int32_t factor)
            {
                UNUSED(output)
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);

                EXPECT_EQ(2, factor);
                clientRunnable->m_outputScaleReceived = true;
            }

            static void OutputHandleDone(void* data, wl_output* output)
            {
                UNUSED(output)
                ClientRunnable* clientRunnable       = static_cast<ClientRunnable*>(data);
                clientRunnable->m_outputDoneReceived = true;
            }

            int                            m_fd = 0;
            std::atomic<bool>              m_outputGeometryReceived;
            std::atomic<bool>              m_outputModeReceived;
            std::atomic<bool>              m_outputScaleReceived;
            std::atomic<bool>              m_outputDoneReceived;
            ramses_internal::ThreadBarrier m_startBarrier{2};

            const struct Registry_Listener : public wl_registry_listener
            {
                Registry_Listener()
                {
                    global        = RegistryHandleGlobalCallBack;
                    global_remove = RegistryHandleGlobalRemoveCallBack;
                }
            } m_registryListener;

            const struct Output_Listener : public wl_output_listener
            {
                Output_Listener()
                {
                    geometry = OutputHandleGeometry;
                    mode     = OutputHandleMode;
                    done     = OutputHandleDone;
                    scale    = OutputHandleScale;
                }
            } m_outputListener;
        };
    }

    class AWaylandOutputResource : public Test
    {
    public:
        virtual void SetUp() override
        {
            m_display = wl_display_create();
            ASSERT_TRUE(m_display != nullptr);
        }

        virtual void TearDown() override
        {
            if (m_client != nullptr)
                wl_client_destroy(m_client);
            wl_display_destroy(m_display);
        }

        void startClientAndDispatchEventsUntilClientHasFinished()
        {
            const int serverFD = m_socket.createBoundFileDescriptor();

            WaylandUtilities::DisplayAddSocketFD(m_display, serverFD);

            wl_global* outputGlobal = wl_global_create(m_display, &wl_output_interface, 2, this, OutputBindCallback);

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
        static void OutputBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
        {
            AWaylandOutputResource* awaylandOutputResource = static_cast<AWaylandOutputResource*>(data);
            wl_resource*          resource = wl_resource_create(client, &wl_output_interface, version, id);
            WaylandOutputResource waylandOutputResource(resource, false);

            switch (awaylandOutputResource->m_sendEventType)
            {
            case ESendEventType_SendGeometry:
                waylandOutputResource.outputSendGeometry(1, 2, 300, 400, WL_OUTPUT_SUBPIXEL_VERTICAL_RGB, "make", "model", WL_OUTPUT_TRANSFORM_90);
                break;
            case ESendEventType_SendMode:
                waylandOutputResource.outputSendMode(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, 300, 400, 60);
                break;
            case ESendEventType_SendScale:
                waylandOutputResource.outputSendScale(2);
                break;
            case ESendEventType_SendDone:
                waylandOutputResource.outputSendDone();
                break;
            }
        }

        wl_display*            m_display = nullptr;
        UnixDomainSocketHelper m_socket  = UnixDomainSocketHelper("testingSocket");
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

    TEST_F(AWaylandOutputResource, CanBeCreatedAndDestroyed)
    {
        const int serverFD = m_socket.createBoundFileDescriptor();
        m_client = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);
        wl_resource* resource = wl_resource_create(m_client, &wl_output_interface, 1, 0);
        WaylandOutputResource waylandOutputResource(resource, true);
    }

    TEST_F(AWaylandOutputResource, CanSendGeometry)
    {
        m_sendEventType = ESendEventType_SendGeometry;
        startClientAndDispatchEventsUntilClientHasFinished();
        EXPECT_TRUE(m_clientRunnable.getOutputGeometryReceived());
    }

    TEST_F(AWaylandOutputResource, CanSendMode)
    {
        m_sendEventType = ESendEventType_SendMode;
        startClientAndDispatchEventsUntilClientHasFinished();
        EXPECT_TRUE(m_clientRunnable.getOutputModeReceived());
    }

    TEST_F(AWaylandOutputResource, CanSendScale)
    {
        m_sendEventType = ESendEventType_SendScale;
        startClientAndDispatchEventsUntilClientHasFinished();
        EXPECT_TRUE(m_clientRunnable.getOutputScaleReceived());
    }

    TEST_F(AWaylandOutputResource, CanSendDone)
    {
        m_sendEventType = ESendEventType_SendDone;
        startClientAndDispatchEventsUntilClientHasFinished();
        EXPECT_TRUE(m_clientRunnable.getOutputDoneReceived());
    }
}
