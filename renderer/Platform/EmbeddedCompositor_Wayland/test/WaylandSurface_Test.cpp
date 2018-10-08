//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "gtest/gtest.h"

#include "WaylandClientMock.h"
#include "WaylandResourceMock.h"
#include "WaylandBufferResourceMock.h"
#include "WaylandCallbackResourceMock.h"
#include "WaylandShellSurfaceMock.h"
#include "WaylandIVISurfaceMock.h"
#include "WaylandBufferMock.h"
#include "EmbeddedCompositor_WaylandMock.h"


namespace ramses_internal
{
    using namespace testing;

    class AWaylandSurface : public Test
    {
    public:
        AWaylandSurface()
        {
        }

        ~AWaylandSurface()
        {
        }

        void createWaylandSurface()
        {
            m_surfaceResource = new WaylandResourceMock;

            const uint32_t id               = 123;
            const uint32_t interfaceVersion = 4;

            InSequence s;
            EXPECT_CALL(m_client, resourceCreate(&wl_surface_interface, interfaceVersion, id))
                .WillOnce(Return(m_surfaceResource));
            EXPECT_CALL(*m_surfaceResource, setImplementation(_, _, _));
            EXPECT_CALL(m_compositor, addWaylandSurface(_));

            m_waylandSurface = new WaylandSurface(m_compositor, m_client, interfaceVersion, id);

            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_FALSE(m_waylandSurface->hasShellSurface());
            EXPECT_EQ(m_waylandSurface->getSurfaceTitle(), "");
            EXPECT_FALSE(m_waylandSurface->hasIviSurface());
            EXPECT_EQ(m_waylandSurface->getIviSurfaceId(), InvalidWaylandIviSurfaceId);
            EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 0u);
        }

        void attachCommitBuffer()
        {
            WaylandBufferResourceMock bufferResource;

            InSequence s;
            EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResource))).WillOnce(ReturnRef(m_waylandBuffer1));
            m_waylandSurface->surfaceAttach(m_client, bufferResource, 0, 0);
            EXPECT_TRUE(m_waylandSurface->hasPendingBuffer());

            EXPECT_CALL(m_waylandBuffer1, reference());
            m_waylandSurface->surfaceCommit(m_client);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        }

        void attachCommitBufferWithIVISurface()
        {
            WaylandBufferResourceMock bufferResource;

            InSequence s;
            EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResource))).WillOnce(ReturnRef(m_waylandBuffer1));
            m_waylandSurface->surfaceAttach(m_client, bufferResource, 0, 0);
            EXPECT_TRUE(m_waylandSurface->hasPendingBuffer());

            EXPECT_CALL(m_waylandBuffer1, reference());
            EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(&m_waylandBuffer1));
            m_waylandSurface->surfaceCommit(m_client);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        }

        void deleteWaylandSurface()
        {
            if (m_waylandSurface != nullptr)
            {
                delete m_waylandSurface;
            }
        }

    protected:

        StrictMock<WaylandClientMock>   m_client;
        StrictMock<EmbeddedCompositor_WaylandMock> m_compositor;
        StrictMock<WaylandShellSurfaceMock> m_shellSurface;
        StrictMock<WaylandIVISurfaceMock> m_iviSurface;
        StrictMock<WaylandBufferMock> m_waylandBuffer1;
        StrictMock<WaylandBufferMock> m_waylandBuffer2;
        WaylandSurface* m_waylandSurface = nullptr;
        WaylandResourceMock* m_surfaceResource = nullptr;
    };

    TEST_F(AWaylandSurface, CanBeCreatedAndDestroyed)
    {
        createWaylandSurface();

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CantBeCreatedWhenResourceCreateFails)
    {
        const uint32_t interfaceVersion = 4;
        const uint32_t id               = 123;

        InSequence s;
        EXPECT_CALL(m_client, resourceCreate(&wl_surface_interface, interfaceVersion, id))
            .WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        EXPECT_CALL(m_compositor, addWaylandSurface(_));

        m_waylandSurface = new WaylandSurface(m_compositor, m_client, interfaceVersion, id);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CanSetShellSurface)
    {
        createWaylandSurface();

        m_waylandSurface->setShellSurface(&m_shellSurface);
        EXPECT_TRUE(m_waylandSurface->hasShellSurface());

        InSequence s;
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_shellSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanSetIVISurface)
    {
        createWaylandSurface();

        m_waylandSurface->setIviSurface(&m_iviSurface);
        EXPECT_TRUE(m_waylandSurface->hasIviSurface());

        InSequence s;
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanGetIVISurfaceId)
    {
        createWaylandSurface();

        m_waylandSurface->setIviSurface(&m_iviSurface);

        const WaylandIviSurfaceId iviSurfaceId(123);

        InSequence s;
        EXPECT_CALL(m_iviSurface, getIviId()).WillOnce(Return(iviSurfaceId));
        EXPECT_EQ(m_waylandSurface->getIviSurfaceId(), iviSurfaceId);

        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanAttachABuffer)
    {
        createWaylandSurface();
        WaylandBufferResourceMock bufferResource;

        InSequence s;
        EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResource))).WillOnce(ReturnRef(m_waylandBuffer1));
        m_waylandSurface->surfaceAttach(m_client, bufferResource, 0, 0);
        EXPECT_TRUE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CanDetachBuffer)
    {
        createWaylandSurface();
        WaylandBufferResourceMock bufferResource;

        InSequence s;
        EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResource))).WillOnce(ReturnRef(m_waylandBuffer1));
        m_waylandSurface->surfaceAttach(m_client, bufferResource, 0, 0);

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanCommitABuffer)
    {
        createWaylandSurface();

        attachCommitBuffer();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_waylandBuffer1,release());
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDetachCommitedBuffer)
    {
        createWaylandSurface();

        InSequence s;
        attachCommitBuffer();

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_waylandBuffer1,release());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CanDetachBufferWhenNoBufferCommitted)
    {
        createWaylandSurface();

        InSequence s;

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanCommitABufferWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        InSequence s;
        attachCommitBufferWithIVISurface();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDetachCommitedBufferWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        InSequence s;
        attachCommitBufferWithIVISurface();

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);

        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CanCommitMultipleBuffersWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        InSequence s;
        attachCommitBufferWithIVISurface();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        WaylandBufferResourceMock bufferResource;
        EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResource))).WillOnce(ReturnRef(m_waylandBuffer2));
        m_waylandSurface->surfaceAttach(m_client, bufferResource, 0, 0);
        EXPECT_TRUE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_waylandBuffer2, reference());
        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(&m_waylandBuffer2));
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer2);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);


        EXPECT_CALL(m_waylandBuffer2, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, SendsFrameCallbackAfterBufferCommitted)
    {
        createWaylandSurface();

        WaylandCallbackResourceMock* callbackResource = new WaylandCallbackResourceMock;

        const uint32_t id(1);
        EXPECT_CALL(m_client, callbackResourceCreate(&wl_callback_interface, 1, id)).WillOnce(Return(callbackResource));
        m_waylandSurface->surfaceFrame(m_client, id);

        attachCommitBuffer();

        const uint32_t time(123);
        EXPECT_CALL(*callbackResource, callbackSendDone(time));
        m_waylandSurface->sendFrameCallbacks(time);

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, SurfaceFramePostsNoMemoryWhenResourceCreateFails)
    {
        createWaylandSurface();

        const uint32_t id(1);
        EXPECT_CALL(m_client, callbackResourceCreate(&wl_callback_interface, 1, id)).WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        m_waylandSurface->surfaceFrame(m_client, id);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, SendsMultipleFrameCallbackAfterBufferCommitted)
    {
        createWaylandSurface();

        WaylandCallbackResourceMock* callbackResource1 = new WaylandCallbackResourceMock;

        const uint32_t id(1);
        EXPECT_CALL(m_client, callbackResourceCreate(&wl_callback_interface, 1, id)).WillOnce(Return(callbackResource1));
        m_waylandSurface->surfaceFrame(m_client, id);

        WaylandCallbackResourceMock* callbackResource2 = new WaylandCallbackResourceMock;

        EXPECT_CALL(m_client, callbackResourceCreate(&wl_callback_interface, 1, id)).WillOnce(Return(callbackResource2));
        m_waylandSurface->surfaceFrame(m_client, id);

        attachCommitBuffer();

        const uint32_t time(123);
        EXPECT_CALL(*callbackResource1, callbackSendDone(time));
        EXPECT_CALL(*callbackResource2, callbackSendDone(time));
        m_waylandSurface->sendFrameCallbacks(time);

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CantCommitWhenNoBufferAttached)
    {
        createWaylandSurface();

        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }


    TEST_F(AWaylandSurface, CanLogInfos)
    {
        createWaylandSurface();

        RendererLogContext logContext(ERendererLogLevelFlag_Details);
        m_waylandSurface->logInfos(logContext);

        EXPECT_STREQ(logContext.getStream().c_str(), "[ivi-surface-id: 4294967295; title: \"\"]\n");

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanLogInfosWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        WaylandIviSurfaceId iviId(123);

        RendererLogContext logContext(ERendererLogLevelFlag_Details);
        EXPECT_CALL(m_iviSurface, getIviId()).WillOnce(Return(iviId));
        m_waylandSurface->logInfos(logContext);

        EXPECT_STREQ(logContext.getStream().c_str(), "[ivi-surface-id: 123; title: \"\"]\n");

        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanLogInfosWithShellSurface)
    {
        createWaylandSurface();
        m_waylandSurface->setShellSurface(&m_shellSurface);

        String title("A Title");

        RendererLogContext logContext(ERendererLogLevelFlag_Details);
        EXPECT_CALL(m_shellSurface, getTitle()).WillOnce(ReturnRef(title));
        m_waylandSurface->logInfos(logContext);

        EXPECT_STREQ(logContext.getStream().c_str(), "[ivi-surface-id: 4294967295; title: \"A Title\"]\n");

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_shellSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanLogInfosWithIVIAndShellSurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);
        m_waylandSurface->setShellSurface(&m_shellSurface);

        WaylandIviSurfaceId iviId(123);
        String              title("A Title");

        RendererLogContext logContext(ERendererLogLevelFlag_Details);
        EXPECT_CALL(m_iviSurface, getIviId()).WillOnce(Return(iviId));
        EXPECT_CALL(m_shellSurface, getTitle()).WillOnce(ReturnRef(title));
        m_waylandSurface->logInfos(logContext);

        EXPECT_STREQ(logContext.getStream().c_str(), "[ivi-surface-id: 123; title: \"A Title\"]\n");

        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(m_shellSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, UnsetsBufferFromSurfaceWhenBufferDestroyed)
    {
        createWaylandSurface();
        attachCommitBuffer();

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->bufferDestroyed(m_waylandBuffer1);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);

        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, UnsetsBufferFromSurfaceWhenBufferDestroyedWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);
        attachCommitBufferWithIVISurface();

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->bufferDestroyed(m_waylandBuffer1);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);

        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanBeDestroyed)
    {
        createWaylandSurface();

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanBeDamaged)
    {
        createWaylandSurface();

        m_waylandSurface->surfaceDamage(m_client, 0, 0, 100, 100);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanSetOpaqueRegion)
    {
        createWaylandSurface();

        WaylandResourceMock regionResource;
        m_waylandSurface->surfaceSetOpaqueRegion(m_client, &regionResource);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanSetInputRegion)
    {
        createWaylandSurface();

        WaylandResourceMock regionResource;
        m_waylandSurface->surfaceSetInputRegion(m_client, &regionResource);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanSetBufferTransform)
    {
        createWaylandSurface();

        WaylandResourceMock regionResource;
        m_waylandSurface->surfaceSetBufferTransform(m_client, 0);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanSetBufferScale)
    {
        createWaylandSurface();

        WaylandResourceMock regionResource;
        m_waylandSurface->surfaceSetBufferScale(m_client, 1);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDamageBuffer)
    {
        createWaylandSurface();

        m_waylandSurface->surfaceDamageBuffer(m_client, 0, 0, 100, 100);

        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
        deleteWaylandSurface();
    }
}
