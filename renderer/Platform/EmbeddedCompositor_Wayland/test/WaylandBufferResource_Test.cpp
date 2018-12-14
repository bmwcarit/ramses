//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#define WL_HIDE_DEPRECATED
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "WaylandUtilities/UnixDomainSocketHelper.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "WaylandUtilities/WaylandUtilities.h"
#include "Utils/ThreadBarrier.h"
#include "Utils/LogMacros.h"
#include "gtest/gtest.h"
#include "wayland-client.h"
#include "wayland-server.h"
#include "sys/mman.h"
#include "fcntl.h"
#include "Utils/File.h"

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        class ClientRunnable : public Runnable
        {
        public:
            ClientRunnable()
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

                // Causes binding against compositor and shm interface in RegistryHandleGlobalCallBack
                wl_display_roundtrip(display);

                assert(nullptr != m_compositor && nullptr != m_shm);

                wl_surface* surface  = wl_compositor_create_surface(m_compositor);

                const uint32_t width  = 10;
                const uint32_t height = 20;
                const uint32_t stride = width * 4;
                const uint32_t size   = stride * height;

                String fileName;
                int fd = CreateAnonymousFile(size, fileName);
                uint8_t* bufferData = static_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));

                for (uint32_t i = 0; i < width * height; i++)
                {
                    bufferData[i] = i;
                }

                wl_shm_pool* pool = wl_shm_create_pool(m_shm, fd, size);
                wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_XRGB8888);

                wl_surface_attach(surface, buffer, 0, 0);

                wl_display_roundtrip(display);

                wl_buffer_destroy(buffer);
                wl_shm_pool_destroy(pool);
                munmap(bufferData, size);
                wl_surface_destroy(surface);
                wl_registry_destroy(registry);
                wl_display_disconnect(display);

                close(fd);
                File file(fileName);
                file.remove();
            }

            void waitOnStart()
            {
                m_startBarrier.wait();
            }

        private:

            int CreateAnonymousFile(off_t size, String& fileName)
            {
                int ret;

                fileName ="SHMBuffer-XXXXXX";

                int fd = mkostemp(const_cast<char*>(fileName.c_str()), O_CLOEXEC);

                if (fd < 0)
                {
                    return -1;
                }

                ret = posix_fallocate(fd, 0, size);
                if (ret != 0)
                {
                    close(fd);
                    return -1;
                }

                return fd;
            }

            static void RegistryHandleGlobalCallBack(
                void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
            {
                ClientRunnable* clientRunnable = static_cast<ClientRunnable*>(data);
                if (strcmp(interface, "wl_compositor") == 0)
                {
                    clientRunnable->m_compositor =
                        static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, version));

                }

                if (strcmp(interface, "wl_shm") == 0)
                {
                    clientRunnable->m_shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
                }
            }

            static void RegistryHandleGlobalRemoveCallBack(void* data, struct wl_registry* registry, uint32_t name)
            {
                UNUSED(data)
                UNUSED(registry)
                UNUSED(name)
            }

            int                            m_fd           = 0;
            ramses_internal::ThreadBarrier m_startBarrier{2};
            wl_compositor*                 m_compositor   = nullptr;
            wl_shm*                        m_shm          = nullptr;

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

    class AWaylandBufferResource : public Test
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
            wl_display_init_shm(m_display);
            const int serverFD = m_socket.createBoundFileDescriptor();

            WaylandUtilities::DisplayAddSocketFD(m_display, serverFD);

            wl_global* compositorGlobal =
                wl_global_create(m_display, &wl_compositor_interface, 1, this, CompositorBindCallback);

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

            wl_global_destroy(compositorGlobal);
        }

    protected:
        static void CompositorBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
        {
            AWaylandBufferResource* awaylandBufferResource = static_cast<AWaylandBufferResource*>(data);
            wl_resource*            resource = wl_resource_create(client, &wl_compositor_interface, version, id);
            wl_resource_set_implementation(resource, &awaylandBufferResource->m_compositorInterface, awaylandBufferResource, nullptr);
        }

        static void CompositorCreateSurfaceCallback(wl_client* client, wl_resource* clientResource, uint32_t id)
        {
            wl_resource* resource = wl_resource_create(client, &wl_surface_interface, 1, id);
            AWaylandBufferResource* awaylandBufferResource = static_cast<AWaylandBufferResource*>(wl_resource_get_user_data(clientResource));
            wl_resource_set_implementation(resource, &awaylandBufferResource->m_surfaceInterface, awaylandBufferResource, nullptr);
        }

        static void SurfaceAttachCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* bufferResource, int x, int y)
        {
            UNUSED(client)
            UNUSED(surfaceResource)
            UNUSED(x)
            UNUSED(y)

            AWaylandBufferResource* awaylandBufferResource = static_cast<AWaylandBufferResource*>(wl_resource_get_user_data(surfaceResource));

            WaylandBufferResource waylandBufferResource(bufferResource, false);
            int32_t width = waylandBufferResource.bufferGetSharedMemoryWidth();

            EXPECT_EQ(10, width);

            int32_t height = waylandBufferResource.bufferGetSharedMemoryHeight();
            EXPECT_EQ(20, height);

            const uint8_t* data = static_cast<const uint8_t*>(waylandBufferResource.bufferGetSharedMemoryData());
            EXPECT_NE(nullptr, data);
            for (int32_t i = 0; i < width * height; i++)
            {
                EXPECT_EQ(i, data[i]);
            }

            awaylandBufferResource->m_surfaceAttachCalled = true;
        }

        const struct Compositor_Interface : public wl_compositor_interface
        {
            Compositor_Interface()
            {
                create_surface = CompositorCreateSurfaceCallback;
            }
        } m_compositorInterface;

        const struct Surface_Interface : public wl_surface_interface
        {
            Surface_Interface()
            {
                attach = SurfaceAttachCallback;
            }
        } m_surfaceInterface;

        static void ResourceDestroyedListener(wl_listener* listener, void* data)
        {
            UNUSED(data)
            WARNINGS_PUSH
            WARNING_DISABLE_LINUX(-Winvalid-offsetof)
            WARNING_DISABLE_LINUX(-Wcast-align)
            WARNING_DISABLE_LINUX(-Wold-style-cast)

            AWaylandBufferResource* waylandBufferResource = wl_container_of(listener, waylandBufferResource, m_destroyListener);
            waylandBufferResource->m_resourceDestroyedListenerCalled++;

            WARNINGS_POP
        }

        wl_display*            m_display = nullptr;
        UnixDomainSocketHelper m_socket  = UnixDomainSocketHelper("testingSocket");
        wl_client*             m_client  = nullptr;
        ClientRunnable         m_clientRunnable;
        bool                   m_surfaceAttachCalled = false;
        wl_listener            m_destroyListener;
        uint32_t               m_resourceDestroyedListenerCalled = 0;
    };

    TEST_F(AWaylandBufferResource, CanBeCreatedAndDestroyed)
    {
        const int serverFD             = m_socket.createBoundFileDescriptor();
        m_client                       = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);

        wl_resource*          resource = wl_resource_create(m_client, &wl_buffer_interface, 1, 0);
        WaylandBufferResource waylandBufferResource(resource, true);
    }

    TEST_F(AWaylandBufferResource, CanGetSharedMemoryWidthHeightAndData)
    {
        startClientAndDispatchEventsUntilClientHasFinished();
        EXPECT_TRUE(m_surfaceAttachCalled);
    }

    TEST_F(AWaylandBufferResource, CantGetSHMBufferDataWhenNoSHMBuffer)
    {
        const int serverFD = m_socket.createBoundFileDescriptor();
        m_client           = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);

        wl_resource*    resource = wl_resource_create(m_client, &wl_buffer_interface, 1, 0);
        WaylandBufferResource waylandBufferResource(resource, true);

        EXPECT_EQ(waylandBufferResource.bufferGetSharedMemoryData(), nullptr);
    }

    TEST_F(AWaylandBufferResource, ReturnsZeroWidthWhenNoSHMBuffer)
    {
        const int serverFD = m_socket.createBoundFileDescriptor();
        m_client           = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);

        wl_resource*    resource = wl_resource_create(m_client, &wl_buffer_interface, 1, 0);
        WaylandBufferResource waylandBufferResource(resource, true);

        EXPECT_EQ(waylandBufferResource.bufferGetSharedMemoryWidth(), 0);
    }

    TEST_F(AWaylandBufferResource, ReturnsZeroHeightWhenNoSHMBuffer)
    {
        const int serverFD = m_socket.createBoundFileDescriptor();
        m_client           = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);

        wl_resource*    resource = wl_resource_create(m_client, &wl_buffer_interface, 1, 0);
        WaylandBufferResource waylandBufferResource(resource, true);

        EXPECT_EQ(waylandBufferResource.bufferGetSharedMemoryHeight(), 0);
    }

    TEST_F(AWaylandBufferResource, CanBeCloned)
    {
        const int serverFD = m_socket.createBoundFileDescriptor();
        m_client           = wl_client_create(m_display, serverFD);
        ASSERT_TRUE(m_client != nullptr);

        wl_resource* resource = wl_resource_create(m_client, &wl_buffer_interface, 1, 0);
        {
            WaylandBufferResource waylandBufferResource(resource, true);
            m_destroyListener.notify = ResourceDestroyedListener;
            waylandBufferResource.addDestroyListener(&m_destroyListener);

            WaylandBufferResource* clonedWaylandBufferResource = waylandBufferResource.clone();

            EXPECT_EQ(resource, clonedWaylandBufferResource->getWaylandNativeResource());
            delete clonedWaylandBufferResource;
            // Cloned object is not owner of the native wayland resource, so destroyed listener must not be called
            EXPECT_EQ(m_resourceDestroyedListenerCalled, 0u);
        }

        EXPECT_EQ(m_resourceDestroyedListenerCalled, 1u);
    }
}
