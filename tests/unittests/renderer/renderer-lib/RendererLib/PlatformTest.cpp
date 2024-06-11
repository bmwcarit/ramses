//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_BaseMock.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "WindowEventHandlerMock.h"
#include "RenderBackendMock.h"

using namespace testing;

namespace ramses::internal
{
    class APlatformTest : public Test
    {
    protected:
        IRenderBackend* createRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDeviceExtension(_));

            EXPECT_CALL(platform, createDevice());

            EXPECT_CALL(platform, createEmbeddedCompositor(_));

            EXPECT_CALL(platform, createTextureUploadingAdapter(_));

            return platform.createRenderBackend(displayConfig, windowEventHandlerMock);
        }

        static IResourceUploadRenderBackend* CreateResourceUploadRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(platform, createContextUploading());
            EXPECT_CALL(*platform.contextUploading, enable()).WillOnce(Return(true));
            EXPECT_CALL(platform, createDeviceUploading());

            return platform.createResourceUploadRenderBackend();
        }

        static void VerifyAndClearExpectationsOnRenderBackendMockObjects(Platform_BaseMock& platform)
        {
            Mock::VerifyAndClearExpectations(platform.window);
            Mock::VerifyAndClearExpectations(platform.context);
            Mock::VerifyAndClearExpectations(platform.device);
            Mock::VerifyAndClearExpectations(platform.embeddedCompositor);
        }

        static void DestroyRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(*platform.context, disable());
            platform.destroyRenderBackend();

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        static void DestroyResourceUploadRenderBackend(Platform_BaseMock& platform)
        {
            InSequence s;
            EXPECT_CALL(*platform.contextUploading, disable());

            platform.destroyResourceUploadRenderBackend();

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        RendererConfigData              rendererConfig;
        DisplayConfigData               displayConfig;
        WindowEventHandlerMock          windowEventHandlerMock;
    };

    TEST_F(APlatformTest, CanCreateAndDestroyRenderBackend)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* renderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != renderBackend);

        DestroyRenderBackend(platform);
    }

    TEST_F(APlatformTest, CanCreateAndDestroyResourceUploadRenderBackend)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);
        IRenderBackend* mainRenderBackend = createRenderBackend(platform);
        ASSERT_TRUE(nullptr != mainRenderBackend);

        IResourceUploadRenderBackend* renderBackend = CreateResourceUploadRenderBackend(platform);
        ASSERT_TRUE(nullptr != renderBackend);

        DestroyResourceUploadRenderBackend(platform);
        DestroyRenderBackend(platform);
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfWindowFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _)).WillOnce(Return(false));

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
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

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfDeviceExtensionFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));

            EXPECT_CALL(platform, createContext(_));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDeviceExtension(_)).WillOnce(Return(false));

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
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

            EXPECT_CALL(platform, createDeviceExtension(_));

            EXPECT_CALL(platform, createDevice()).WillOnce(Return(false)); //device fails init
            EXPECT_CALL(*platform.context, disable());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
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

            EXPECT_CALL(platform, createDeviceExtension(_));

            EXPECT_CALL(platform, createDevice());

            EXPECT_CALL(platform, createEmbeddedCompositor(_)).WillOnce(Return(false)); //embedded compositor fails init
            EXPECT_CALL(*platform.context, disable());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
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

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        DestroyRenderBackend(platform);
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

            VerifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        DestroyRenderBackend(platform);
    }
}
