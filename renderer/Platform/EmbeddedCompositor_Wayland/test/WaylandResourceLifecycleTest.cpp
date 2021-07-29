//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandUtilities/UnixDomainSocket.h"
#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "EmbeddedCompositor_WaylandMock.h"
#include "WaylandClientMock.h"
#include "NativeWaylandResourceMock.h"
#include "WaylandSurfaceMock.h"
#include "ContextMock.h"

#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandShellSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandIVISurface.h"
#include "EmbeddedCompositor_Wayland/WaylandRegion.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufParams.h"
#include "EmbeddedCompositor_Wayland/WaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandIVIApplicationConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandShellConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputConnection.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufConnection.h"

#include "RendererLib/RendererConfig.h"
#include "RendererLib/DisplayConfig.h"

#include "gtest/gtest.h"


namespace ramses_internal
{
    using namespace testing;

    template<typename ResourceT>
    class AWaylandResourceLifecycleTest : public ::testing::Test
    {
    protected:
        virtual void SetUp() override
        {
            //used for simulating cases where a Mock object is not enough, then a fake wl_resource object is used
            createFakeWaylandFixtures();

            m_nativeResourceMock = new StrictMock<NativeWaylandResourceMockWithDestructor>;

            //store the params used for creation of WaylandResource to check they are the same at destruction
            EXPECT_CALL(m_clientMock, resourceCreate(_, _, _)).WillOnce(DoAll(SaveArg<0>(&m_createdInterface), Return(m_nativeResourceMock)));
            EXPECT_CALL(*m_nativeResourceMock, setImplementation(_, _, _)).WillOnce(DoAll(SaveArg<0>(&m_createdImplementation), SaveArg<1>(&m_createdUserData), SaveArg<2>(&m_resourceDestroyedCallback)));

            m_resourceWithImplementation = createResourceWithImplementation();
            EXPECT_EQ(m_createdUserData, m_resourceWithImplementation);

            //fake the user data of the fake wl_resource, to be used in tests for destruction of wl_resource
            wl_resource_set_user_data(m_fakeWaylandResource, m_resourceWithImplementation);
        }

        virtual void TearDown() override
        {
            destroyFakeWaylandFixtures();
        }

        void createFakeWaylandFixtures()
        {
            m_fakeWaylandDisplay = wl_display_create();
            ASSERT_TRUE(m_fakeWaylandDisplay != nullptr);
            const int serverFD = m_fakeTestingSocket.createBoundFileDescriptor();
            ASSERT_NE(serverFD, -1);
            m_fakeWaylandClient = wl_client_create(m_fakeWaylandDisplay, serverFD);
            ASSERT_TRUE(m_fakeWaylandClient != nullptr);
            m_fakeWaylandResource = wl_resource_create(m_fakeWaylandClient, nullptr, 0, 0u);
            ASSERT_TRUE(m_fakeWaylandResource != nullptr);
        }

        void destroyFakeWaylandFixtures()
        {
            if(m_fakeWaylandResource)
                wl_resource_destroy(m_fakeWaylandResource);

            if(m_fakeWaylandClient)
                wl_client_destroy(m_fakeWaylandClient);

            if(m_fakeWaylandDisplay)
                wl_display_destroy(m_fakeWaylandDisplay);
        }

        void expectNativeResourceWillBeDeleted(bool expectLowLevelResourceDestroy)
        {
            InSequence s;
            if(expectLowLevelResourceDestroy)
            {
                //Check resrouce destroy func is reset (and first two params not changed)
                EXPECT_CALL(*this->m_nativeResourceMock, setImplementation(this->m_createdImplementation, this->m_createdUserData, nullptr));
                EXPECT_CALL(*this->m_nativeResourceMock, destroy());
            }
            //Check resource wrapper gets destroyed
            EXPECT_CALL(*this->m_nativeResourceMock, Die());
        }

        ResourceT* createResourceWithImplementation();

        //fixtures needed for creation of fake wayland resource
        UnixDomainSocket m_fakeTestingSocket = UnixDomainSocket("testingSocket", WaylandEnvironmentUtils::GetVariable(WaylandEnvironmentVariable::XDGRuntimeDir));
        wl_display* m_fakeWaylandDisplay = nullptr;
        wl_client* m_fakeWaylandClient = nullptr;
        wl_resource* m_fakeWaylandResource = nullptr;

        //some objects are needed for creation of some of test fixtures, but these tests
        //need to focus on other aspects of resource lifecycle
        NiceMock<EmbeddedCompositor_WaylandMock>        m_compositorMock;
        NiceMock<WaylandSurfaceMock>                    m_surfaceMock;
        NiceMock<ContextMock>                           m_contextMock;
        EmbeddedCompositor_Wayland                      m_waylandCompositor{{}, m_contextMock};

        StrictMock<WaylandClientMock>                   m_clientMock;
        StrictMock<NativeWaylandResourceMockWithDestructor>*  m_nativeResourceMock = nullptr;
        ResourceT*                                      m_resourceWithImplementation = nullptr;

        //Values extracted during resource creation in the test to be used later in expectations
        const wl_interface* m_createdInterface = nullptr;
        const void*         m_createdImplementation = nullptr;
        void*               m_createdUserData = nullptr;
        std::function<void(wl_resource*)> m_resourceDestroyedCallback;
    };

    using TestTypes = ::testing::Types<
        WaylandSurface,
        WaylandShellSurface,
        WaylandIVISurface,
        WaylandRegion,
        LinuxDmabufParams,
        WaylandCompositorConnection,
        WaylandIVIApplicationConnection,
        WaylandShellConnection,
        WaylandOutputConnection,
        LinuxDmabufConnection
    >;
    TYPED_TEST_SUITE(AWaylandResourceLifecycleTest, TestTypes);

    template<>
    WaylandSurface* AWaylandResourceLifecycleTest<WaylandSurface>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new WaylandSurface(m_compositorMock, m_clientMock, interfaceVersion, id);
    }

    template<>
    WaylandShellSurface* AWaylandResourceLifecycleTest<WaylandShellSurface>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        NiceMock<NativeWaylandResourceMock> shellConnectionResourceMock;
        return new WaylandShellSurface(m_clientMock, shellConnectionResourceMock, id, m_surfaceMock);
    }

    template<>
    WaylandIVISurface* AWaylandResourceLifecycleTest<WaylandIVISurface>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        NiceMock<NativeWaylandResourceMock> iviConnectionResourceMock;
        const WaylandIviSurfaceId iviId { 123u };
        return new WaylandIVISurface(m_clientMock, iviConnectionResourceMock, iviId, &m_surfaceMock, id, m_waylandCompositor);
    }

    template<>
    WaylandRegion* AWaylandResourceLifecycleTest<WaylandRegion>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new WaylandRegion(m_compositorMock, m_clientMock, interfaceVersion, id);
    }

    template<>
    LinuxDmabufParams* AWaylandResourceLifecycleTest<LinuxDmabufParams>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new LinuxDmabufParams(m_clientMock, interfaceVersion, id);
    }

    template<>
    WaylandCompositorConnection* AWaylandResourceLifecycleTest<WaylandCompositorConnection>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new WaylandCompositorConnection(m_clientMock, interfaceVersion, id, m_compositorMock);
    }

    template<>
    WaylandIVIApplicationConnection* AWaylandResourceLifecycleTest<WaylandIVIApplicationConnection>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new WaylandIVIApplicationConnection(m_clientMock, interfaceVersion, id, m_waylandCompositor);
    }

    template<>
    WaylandShellConnection* AWaylandResourceLifecycleTest<WaylandShellConnection>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 123;
        constexpr uint32_t interfaceVersion = 4;
        return new WaylandShellConnection(m_clientMock, interfaceVersion, id);
    }

    template<>
    WaylandOutputConnection* AWaylandResourceLifecycleTest<WaylandOutputConnection>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 888;
        constexpr uint32_t interfaceVersion = 1;

        //replace fake wl_resource by another fake wl_resource that implements wl_output_protocol as this is needed for the test
        wl_resource_destroy(m_fakeWaylandResource);
        m_fakeWaylandResource = wl_resource_create(m_fakeWaylandClient, &wl_output_interface, 0, 0u);
        EXPECT_NE(m_fakeWaylandResource, nullptr);
        EXPECT_CALL(*m_nativeResourceMock, getLowLevelHandle()).WillOnce(Return(m_fakeWaylandResource));

        return new WaylandOutputConnection({}, m_clientMock, interfaceVersion, id);
    }

    template<>
    LinuxDmabufConnection* AWaylandResourceLifecycleTest<LinuxDmabufConnection>::createResourceWithImplementation()
    {
        constexpr uint32_t id = 888;
        constexpr uint32_t interfaceVersion = 1;
        return new LinuxDmabufConnection(m_clientMock, interfaceVersion, id);
    }

    TYPED_TEST(AWaylandResourceLifecycleTest, CanBeExplicitlyDeallocated)
    {
        this->expectNativeResourceWillBeDeleted(true);
        delete this->m_resourceWithImplementation;

        Mock::VerifyAndClearExpectations(this->m_nativeResourceMock);
    }

    TYPED_TEST(AWaylandResourceLifecycleTest, CanBeDeallocatedUsingResorceDestroyCallback)
    {
        this->expectNativeResourceWillBeDeleted(false);
        //initiate the resource destoy sequence using the callback with the fake resource (which returns the object being tested as user data (see SetUp()))
        this->m_resourceDestroyedCallback(this->m_fakeWaylandResource);

        Mock::VerifyAndClearExpectations(this->m_nativeResourceMock);
    }
}
