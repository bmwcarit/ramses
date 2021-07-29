//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "Platform_BaseMock.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "WindowEventHandlerMock.h"
#include "RenderBackendMock.h"
#include "Utils/ThreadLocalLog.h"

using namespace testing;

namespace ramses_internal
{
    class APlatformTest : public Test
    {
    public:
        static void SetUpTestCase()
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

    protected:
        IRenderBackend* createRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice());

            EXPECT_CALL(platform, createEmbeddedCompositor(_));

            EXPECT_CALL(platform, createTextureUploadingAdapter(_));

            return platform.createRenderBackend(displayConfig, windowEventHandlerMock);
        }

        IResourceUploadRenderBackend* createResourceUploadRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(platform, createContextUploading());
            EXPECT_CALL(*platform.contextUploading, enable()).WillOnce(Return(true));
            EXPECT_CALL(platform, createDeviceUploading());

            return platform.createResourceUploadRenderBackend();
        }

        void verifyAndClearExpectationsOnRenderBackendMockObjects(Platform_BaseMock& platform)
        {
            Mock::VerifyAndClearExpectations(platform.window);
            Mock::VerifyAndClearExpectations(platform.context);
            Mock::VerifyAndClearExpectations(platform.device);
            Mock::VerifyAndClearExpectations(platform.embeddedCompositor);
        }

        void destroyRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(*platform.context, disable());
            platform.destroyRenderBackend();

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        void destroyResourceUploadRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(*platform.contextUploading, disable());

            platform.destroyResourceUploadRenderBackend();

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        RendererConfig                  rendererConfig;
        DisplayConfig                   displayConfig;
        WindowEventHandlerMock          windowEventHandlerMock;
    };

    TEST_F(APlatformTest, CanCreateAndDestroyRenderBackend)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* renderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != renderBackend);

        destroyRenderBackend(platform);
    }

    TEST_F(APlatformTest, CanCreateAndDestroyResourceUploadRenderBackend)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* mainRenderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != mainRenderBackend);

        IResourceUploadRenderBackend* renderBackend = createResourceUploadRenderBackend(platform);
        ASSERT_TRUE(nullptr != renderBackend);

        destroyResourceUploadRenderBackend(platform);
        destroyRenderBackend(platform);
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfWindowFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _)).WillOnce(Return(false));

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfContextFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_)).WillOnce(Return(false)); //context fails init

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfDeviceFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice()).WillOnce(Return(false)); //device fails init
            EXPECT_CALL(*platform.context, disable());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }


    TEST_F(APlatformTest, RenderBackendCreationFailsIfEmbeddedCompositorFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice());

            EXPECT_CALL(platform, createEmbeddedCompositor(_)).WillOnce(Return(false)); //embedded compositor fails init
            EXPECT_CALL(*platform.context, disable());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }

    TEST_F(APlatformTest, ResourceUploadRenderBackendCreationFailsIfContextFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* mainRenderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != mainRenderBackend);

        {
            InSequence s;
            EXPECT_CALL(platform, createContextUploading()).WillOnce(Return(false));

            IResourceUploadRenderBackend* renderBackend = platform.createResourceUploadRenderBackend();
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        destroyRenderBackend(platform);
    }

    TEST_F(APlatformTest, ResourceUploadRenderBackendCreationFailsIfDeviceFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* mainRenderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != mainRenderBackend);

        {
            InSequence s;
            EXPECT_CALL(platform, createContextUploading());
            EXPECT_CALL(*platform.contextUploading, enable()).WillOnce(Return(true));
            EXPECT_CALL(platform, createDeviceUploading()).WillOnce(Return(false));
            EXPECT_CALL(*platform.contextUploading, disable());

            IResourceUploadRenderBackend* renderBackend = platform.createResourceUploadRenderBackend();
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        destroyRenderBackend(platform);
    }
}
