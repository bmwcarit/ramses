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
#include "NativeWaylandResourceMock.h"
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

        ~AWaylandSurface() override
        {
        }

        void createWaylandSurface()
        {
            m_surfaceResource = new StrictMock<NativeWaylandResourceMock>;

            const uint32_t id               = 123;
            const uint32_t interfaceVersion = 4;

            EXPECT_CALL(m_client, resourceCreate(&wl_surface_interface, interfaceVersion, id))
                .WillOnce(Return(m_surfaceResource));
            EXPECT_CALL(*m_surfaceResource, setImplementation(_, _, _));
            EXPECT_CALL(m_compositor, addWaylandSurface(_));

            m_waylandSurface = new StrictMock<WaylandSurface>(m_compositor, m_client, interfaceVersion, id);

            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_FALSE(m_waylandSurface->hasShellSurface());
            EXPECT_EQ(m_waylandSurface->getSurfaceTitle(), "");
            EXPECT_FALSE(m_waylandSurface->hasIviSurface());
            EXPECT_FALSE(m_waylandSurface->getIviSurfaceId().isValid());
            EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 0u);
            EXPECT_FALSE(m_waylandSurface->dispatchBufferTypeChanged());
        }

        void attachBuffer(WaylandBufferResourceMock& bufferResourceMock, WaylandBufferMock& bufferMock, std::initializer_list<std::pair<WaylandBufferMock&, bool>> bufferTypeExpecations)
        {
            EXPECT_CALL(m_compositor, getOrCreateBuffer(Ref(bufferResourceMock))).WillOnce(ReturnRef(bufferMock));
            for(auto& expectation: bufferTypeExpecations)
            {
                EXPECT_CALL(expectation.first, isSharedMemoryBuffer()).WillOnce(Return(expectation.second));
            }
            EXPECT_CALL(m_iviSurface, getIviId()).Times(AtMost(1)); //in case an ivi-surface is set

            m_waylandSurface->surfaceAttach(m_client, bufferResourceMock, 0, 0);
            EXPECT_TRUE(m_waylandSurface->hasPendingBuffer());
        }

        void commitBuffer(WaylandBufferMock& bufferMock, WaylandBufferMock* bufferMockToRelease = nullptr)
        {
            EXPECT_CALL(bufferMock, reference());
            if(bufferMockToRelease)
                EXPECT_CALL(*bufferMockToRelease,release());

            m_waylandSurface->surfaceCommit(m_client);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &bufferMock);
        }
        void attachCommitBuffer()
        {
            WaylandBufferResourceMock bufferResource;
            attachBuffer(bufferResource, m_waylandBuffer1, { {m_waylandBuffer1, false} });
            commitBuffer(m_waylandBuffer1);
        }

        void attachCommitBufferWithIVISurface()
        {
            WaylandBufferResourceMock bufferResource;
            attachBuffer(bufferResource, m_waylandBuffer1, {{m_waylandBuffer1, false}});

            EXPECT_CALL(m_waylandBuffer1, reference());
            EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(&m_waylandBuffer1));
            m_waylandSurface->surfaceCommit(m_client);
            EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
            EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        }

        void deleteWaylandSurface(bool hasIviSurface = false, bool hasShellSurface = false)
        {
            if(hasIviSurface)
            {
                EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
            }

            EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));

            if(hasIviSurface)
            {
                EXPECT_CALL(m_iviSurface, surfaceWasDeleted());
            }

            if(hasShellSurface)
            {
                EXPECT_CALL(m_shellSurface, surfaceWasDeleted());
            }

            EXPECT_CALL(*m_surfaceResource, setImplementation(_, m_waylandSurface, nullptr));
            EXPECT_CALL(*m_surfaceResource, destroy());

            delete m_waylandSurface;
        }

        void expectSurfaceBufferTypeChanged(bool changed)
        {
            if(changed)
            {
                EXPECT_TRUE(m_waylandSurface->dispatchBufferTypeChanged());
                //flag gets reset after dispatch
                EXPECT_FALSE(m_waylandSurface->dispatchBufferTypeChanged());
            }
            else
            {
                EXPECT_FALSE(m_waylandSurface->dispatchBufferTypeChanged());
            }
        }

    protected:
        InSequence m_testSequence;

        StrictMock<WaylandClientMock>   m_client;
        StrictMock<EmbeddedCompositor_WaylandMock> m_compositor;
        StrictMock<WaylandShellSurfaceMock> m_shellSurface;
        StrictMock<WaylandIVISurfaceMock> m_iviSurface;
        StrictMock<WaylandBufferMock> m_waylandBuffer1;
        StrictMock<WaylandBufferMock> m_waylandBuffer2;
        StrictMock<WaylandSurface>* m_waylandSurface = nullptr;
        StrictMock<NativeWaylandResourceMock>* m_surfaceResource = nullptr;
    };

    TEST_F(AWaylandSurface, CanBeCreatedAndDestroyed)
    {
        createWaylandSurface();
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CantBeCreatedWhenResourceCreateFails)
    {
        const uint32_t interfaceVersion = 4;
        const uint32_t id               = 123;

        EXPECT_CALL(m_client, resourceCreate(&wl_surface_interface, interfaceVersion, id))
            .WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        EXPECT_CALL(m_compositor, addWaylandSurface(_));

        m_waylandSurface = new StrictMock<WaylandSurface>(m_compositor, m_client, interfaceVersion, id);


        EXPECT_CALL(m_compositor, removeWaylandSurface(Ref(*m_waylandSurface)));
        delete m_waylandSurface;
        //destory resource explicitly to avoid leak
        delete m_surfaceResource;
    }

    TEST_F(AWaylandSurface, CanSetShellSurface)
    {
        createWaylandSurface();

        m_waylandSurface->setShellSurface(&m_shellSurface);
        EXPECT_TRUE(m_waylandSurface->hasShellSurface());

        deleteWaylandSurface(false, true);
    }

    TEST_F(AWaylandSurface, CanSetIVISurface)
    {
        createWaylandSurface();

        m_waylandSurface->setIviSurface(&m_iviSurface);
        EXPECT_TRUE(m_waylandSurface->hasIviSurface());

        deleteWaylandSurface(true);
    }

    TEST_F(AWaylandSurface, CanGetIVISurfaceId)
    {
        createWaylandSurface();

        m_waylandSurface->setIviSurface(&m_iviSurface);

        const WaylandIviSurfaceId iviSurfaceId(123);

        EXPECT_CALL(m_iviSurface, getIviId()).WillOnce(Return(iviSurfaceId));
        EXPECT_EQ(m_waylandSurface->getIviSurfaceId(), iviSurfaceId);

        deleteWaylandSurface(true);
    }

    TEST_F(AWaylandSurface, CanAttachABuffer)
    {
        createWaylandSurface();
        WaylandBufferResourceMock bufferResource;

        attachBuffer(bufferResource, m_waylandBuffer1, {{m_waylandBuffer1, false}});

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDetachBuffer)
    {
        createWaylandSurface();
        WaylandBufferResourceMock bufferResource;

        attachBuffer(bufferResource, m_waylandBuffer1, {{m_waylandBuffer1, false}});

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanCommitABuffer)
    {
        createWaylandSurface();

        attachCommitBuffer();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_waylandBuffer1,release());
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDetachCommitedBuffer)
    {
        createWaylandSurface();

        attachCommitBuffer();

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_waylandBuffer1,release());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanDetachBufferWhenNoBufferCommitted)
    {
        createWaylandSurface();

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanCommitABufferWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);
        attachCommitBufferWithIVISurface();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        EXPECT_CALL(m_waylandBuffer1, release());
        deleteWaylandSurface(true);
    }

    TEST_F(AWaylandSurface, CanDetachCommitedBufferWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        attachCommitBufferWithIVISurface();

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());

        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(nullptr));
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer1);
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);

        deleteWaylandSurface(true);
    }


    TEST_F(AWaylandSurface, CanCommitMultipleBuffersWithIVISurface)
    {
        createWaylandSurface();
        m_waylandSurface->setIviSurface(&m_iviSurface);

        attachCommitBufferWithIVISurface();
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);
        expectSurfaceBufferTypeChanged(true);

        WaylandBufferResourceMock bufferResource;
        attachBuffer(bufferResource, m_waylandBuffer2, {{m_waylandBuffer2, true}, {m_waylandBuffer1, true}});
        expectSurfaceBufferTypeChanged(false);

        EXPECT_CALL(m_waylandBuffer2, reference());
        EXPECT_CALL(m_waylandBuffer1, release());
        EXPECT_CALL(m_iviSurface, bufferWasSetToSurface(&m_waylandBuffer2));
        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), &m_waylandBuffer2);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 2u);


        EXPECT_CALL(m_waylandBuffer2, release());
        deleteWaylandSurface(true);
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
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, SurfaceFramePostsNoMemoryWhenResourceCreateFails)
    {
        createWaylandSurface();

        const uint32_t id(1);
        EXPECT_CALL(m_client, callbackResourceCreate(&wl_callback_interface, 1, id)).WillOnce(Return(nullptr));
        EXPECT_CALL(m_client, postNoMemory());
        m_waylandSurface->surfaceFrame(m_client, id);

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
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CantCommitWhenNoBufferAttached)
    {
        createWaylandSurface();

        m_waylandSurface->surfaceCommit(m_client);
        EXPECT_FALSE(m_waylandSurface->hasPendingBuffer());
        EXPECT_EQ(m_waylandSurface->getWaylandBuffer(), nullptr);
        EXPECT_EQ(m_waylandSurface->getNumberOfCommitedFrames(), 1u);

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, CanLogInfos)
    {
        createWaylandSurface();

        RendererLogContext logContext(ERendererLogLevelFlag_Details);
        m_waylandSurface->logInfos(logContext);

        EXPECT_STREQ(logContext.getStream().c_str(), "[ivi-surface-id: 4294967295; title: \"\"]\n");

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

        deleteWaylandSurface(true);
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

        deleteWaylandSurface(false, true);
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

        deleteWaylandSurface(true, true);
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

        deleteWaylandSurface(true);
    }

    TEST_F(AWaylandSurface, MarksBufferTypeChanged_IfFirstBufferAttached)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource;
        attachBuffer(bufferResource, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, DoesNotBufferTypeChanged_IfBufferDetached)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource;
        attachBuffer(bufferResource, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        //if buffer detached (null buffer attached) this means no content will be rendered using embedded compositor
        //until next attach, so swizzle does not have to be reset
        m_waylandSurface->surfaceDetach(m_client);
        expectSurfaceBufferTypeChanged(false);
        EXPECT_EQ(nullptr, m_waylandSurface->getWaylandBuffer()); //no content will be rendered anyway so it is safe to not reset swizzle

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, DoesNotMarkBufferTypeChanged_IfBufferAttachedWithSameTypeAfterDetachWithoutCommit)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        commitBuffer(m_waylandBuffer1);
        expectSurfaceBufferTypeChanged(false); //commit does not change buffer type by itself

        m_waylandSurface->surfaceDetach(m_client);

        WaylandBufferResourceMock bufferResource2;
        //detach resets only "pending buffer", while "current buffer" is reset only on commit
        //therefore, if a detach is followed by an attach within the same commit then the detach
        //is a no-op, i.e., it is safe to not reset swizzle if current and pending (from the new attach
        //after detach) buffer are of same type
        attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2, true}, {m_waylandBuffer1, true}});
        expectSurfaceBufferTypeChanged(false);

        EXPECT_CALL(m_waylandBuffer1, release());
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, MarksBufferTypeChanged_IfBufferAttachedWithDifferentTypeAfterDetachWithoutCommit)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        commitBuffer(m_waylandBuffer1);
        expectSurfaceBufferTypeChanged(false); //commit does not change buffer type by itself

        m_waylandSurface->surfaceDetach(m_client);

        WaylandBufferResourceMock bufferResource2;
        attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2, true}, {m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        EXPECT_CALL(m_waylandBuffer1, release());
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, MarksBufferTypeChanged_IfBufferAttachedAfterDetachWithCommit)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        commitBuffer(m_waylandBuffer1);
        expectSurfaceBufferTypeChanged(false); //commit does not change buffer type by itself

        m_waylandSurface->surfaceDetach(m_client);
        EXPECT_CALL(m_waylandBuffer1, release());
        m_waylandSurface->surfaceCommit(m_client);
        expectSurfaceBufferTypeChanged(false); //commit does not change buffer type by itself


        //it does not matter if new buffer has same or different type as last buffer
        //the buffer type will be marked as changed anyway, which might be more than
        //necessary but not wrong to do, i.e., theoricially it is more optimal to store
        //the type of the last buffer received and always check and update it so after
        //a detach this information is not lost
        WaylandBufferResourceMock bufferResource2;
        attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2, false}});
        expectSurfaceBufferTypeChanged(true);

        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, MarksBufferTypeChanged_IfBufferOfDifferentTypeAttached)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        WaylandBufferResourceMock bufferResource2;
        WaylandBufferResourceMock bufferResource3;

        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        {
            //pending buffer becomes current buffer
            commitBuffer(m_waylandBuffer1);
            //attach new pending buffer (1st is SHM, 2nd is EGL)
            attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2 , true} , {m_waylandBuffer1, false}});
            expectSurfaceBufferTypeChanged(true);

        }

        {
            //pending buffer becomes current buffer, and current buffer is released
            commitBuffer(m_waylandBuffer2, &m_waylandBuffer1);
            //attach new pending buffer (1st is EGL, 2nd is SHM)
            attachBuffer(bufferResource3, m_waylandBuffer1, {{m_waylandBuffer1 , false} , {m_waylandBuffer2, true}});
            expectSurfaceBufferTypeChanged(true);

        }

        EXPECT_CALL(m_waylandBuffer2, release());
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, DoesNotMarkBufferTypeChanged_IfBufferOfSameTypeAttached_BothSHM)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        WaylandBufferResourceMock bufferResource2;

        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        //pending buffer becomes current buffer
        commitBuffer(m_waylandBuffer1);
        //attach new pending buffer (both SHM)
        attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2 , true} , {m_waylandBuffer1, true}});
        expectSurfaceBufferTypeChanged(false);

        EXPECT_CALL(m_waylandBuffer1, release());
        deleteWaylandSurface();
    }

    TEST_F(AWaylandSurface, DoesNotMarkBufferTypeChanged_IfBufferOfSameTypeAttached_BothEGL)
    {
        createWaylandSurface();

        WaylandBufferResourceMock bufferResource1;
        WaylandBufferResourceMock bufferResource2;

        attachBuffer(bufferResource1, m_waylandBuffer1, {{m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(true);

        //pending buffer becomes current buffer
        commitBuffer(m_waylandBuffer1);
        //attach new pending buffer (both EGL)
        attachBuffer(bufferResource2, m_waylandBuffer2, {{m_waylandBuffer2 , false} , {m_waylandBuffer1, false}});
        expectSurfaceBufferTypeChanged(false);

        EXPECT_CALL(m_waylandBuffer1, release());
        deleteWaylandSurface();
    }
}
