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

using namespace testing;

namespace ramses_internal
{
    class APlatformTest : public Test
    {
    protected:
        IRenderBackend* createRenderBackend(Platform_BaseMock& platform)
        {
            platform.createRenderBackendMockObjects();

            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));
            EXPECT_CALL(*platform.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createContext(Ref(*platform.window)));
            EXPECT_CALL(*platform.context, init()).WillOnce(Return(true));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice(Ref(*platform.context)));
            EXPECT_CALL(*platform.device, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createEmbeddedCompositor(_, Ref(*platform.context)));
            EXPECT_CALL(*platform.embeddedCompositor, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createTextureUploadingAdapter(Ref(*platform.device), Ref(*platform.embeddedCompositor), Ref(*platform.window)));

            return platform.createRenderBackend(displayConfig, windowEventHandlerMock);
        }

        void createPerRendererComponents(Platform_BaseMock& platform, Bool initSystemCompositorControllerSucess = true)
        {
            platform.systemCompositorController = platform.createMockObjectHelper<SystemCompositorControllerMockWithDestructor>();

            InSequence s;
            EXPECT_CALL(platform, createSystemCompositorController());
            EXPECT_CALL(*platform.systemCompositorController, init()).WillOnce(Return(initSystemCompositorControllerSucess));

            if(initSystemCompositorControllerSucess)
            {
                platform.createPerRendererComponents();

            }
            else
            {
                EXPECT_CALL(static_cast<SystemCompositorControllerMockWithDestructor&>(*platform.systemCompositorController), Die()); //expect destructor called

                platform.createPerRendererComponents();

                Mock::VerifyAndClearExpectations(platform.systemCompositorController);
                platform.systemCompositorController = nullptr;
            }
        }

        void verifyAndClearExpectationsOnRenderBackendMockObjects(Platform_BaseMock& platform)
        {
            Mock::VerifyAndClearExpectations(platform.window);
            Mock::VerifyAndClearExpectations(platform.context);
            Mock::VerifyAndClearExpectations(platform.device);
            Mock::VerifyAndClearExpectations(platform.embeddedCompositor);
        }

        void destroyRenderBackend(Platform_BaseMock& platform, IRenderBackend& renderBackend)
        {
            InSequence s;
            EXPECT_CALL(*platform.embeddedCompositor, Die());
            EXPECT_CALL(*platform.device, Die());
            EXPECT_CALL(*platform.context, disable());
            EXPECT_CALL(*platform.context, Die());
            EXPECT_CALL(*platform.window, Die());

            platform.destroyRenderBackend(renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);

            platform.window                  = nullptr;
            platform.context                 = nullptr;
            platform.device                  = nullptr;
            platform.embeddedCompositor      = nullptr;
        }

        void destroyPerRendererComponents(Platform_BaseMock& platform)
        {
            EXPECT_CALL(static_cast<SystemCompositorControllerMockWithDestructor&>(*platform.systemCompositorController), Die());
            platform.destroyPerRendererComponents();

            Mock::VerifyAndClearExpectations(platform.systemCompositorController);

            platform.systemCompositorController = nullptr;
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

        destroyRenderBackend(platform, *renderBackend);
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfWindowFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));
            EXPECT_CALL(*platform.window, init()).WillOnce(Return(false)); //window fails init
            //destructor gets called
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platform.window), Die());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platform.embeddedCompositor), Die());
        EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platform.device), Die());
        EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), Die());
        delete platform.context;
        delete platform.device;
        delete platform.embeddedCompositor;
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfContextFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));
            EXPECT_CALL(*platform.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createContext(Ref(*platform.window)));
            EXPECT_CALL(*platform.context, init()).WillOnce(Return(false)); //context fails init
            //destructors gets called
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platform.window), Die());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platform.embeddedCompositor), Die());
        EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platform.device), Die());
        delete platform.device;
        delete platform.embeddedCompositor;
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfDeviceFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));
            EXPECT_CALL(*platform.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createContext(Ref(*platform.window)));
            EXPECT_CALL(*platform.context, init()).WillOnce(Return(true));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice(Ref(*platform.context)));
            EXPECT_CALL(*platform.device, init()).WillOnce(Return(false)); //device fails init

            //destructors gets called
            EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platform.device), Die());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), disable());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platform.window), Die());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platform.embeddedCompositor), Die());
        delete platform.embeddedCompositor;
    }


    TEST_F(APlatformTest, RenderBackendCreationFailsIfEmbeddedCompositorFailsInitialization)
    {
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platform, createWindow(_, _));
            EXPECT_CALL(*platform.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createContext(Ref(*platform.window)));
            EXPECT_CALL(*platform.context, init()).WillOnce(Return(true));

            EXPECT_CALL(*platform.context, enable()).WillOnce(Return(true));

            EXPECT_CALL(platform, createDevice(Ref(*platform.context)));
            EXPECT_CALL(*platform.device, init()).WillOnce(Return(true));

            EXPECT_CALL(platform, createEmbeddedCompositor(_, Ref(*platform.context)));
            EXPECT_CALL(*platform.embeddedCompositor, init()).WillOnce(Return(false)); //embedded compositor fails init

            //destructors gets called
            EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platform.embeddedCompositor), Die());
            EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platform.device), Die());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), disable());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platform.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platform.window), Die());

            IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platform);
        }
    }

    TEST_F(APlatformTest, CanCreateAndDestroyPerRendererComponents_WithoutSystemCompositorController)
    {
        ASSERT_FALSE(rendererConfig.getSystemCompositorControlEnabled());
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createPerRendererComponents();

        EXPECT_EQ(nullptr, platform.getSystemCompositorController());
        platform.destroyPerRendererComponents();
    }

    TEST_F(APlatformTest, CanCreateRenderBackendIfSystemCompositorControllerIsNotEnabled)
    {
        ASSERT_FALSE(rendererConfig.getSystemCompositorControlEnabled());
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        platform.createPerRendererComponents();

        EXPECT_EQ(nullptr, platform.getSystemCompositorController());

        IRenderBackend* renderBackend = createRenderBackend(platform);
        EXPECT_NE(nullptr, renderBackend);
        destroyRenderBackend(platform, *renderBackend);

        platform.destroyPerRendererComponents();
    }

    TEST_F(APlatformTest, CanCreateAndDestroyPerRendererComponents_WithSystemCompositorController)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        createPerRendererComponents(platform);

        EXPECT_NE(nullptr, platform.getSystemCompositorController());
        EXPECT_EQ(platform.systemCompositorController, platform.getSystemCompositorController());
        destroyPerRendererComponents(platform);
    }

    TEST_F(APlatformTest, DestroysSystemCompositorIfItFailsInitialization)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        createPerRendererComponents(platform, false);
        EXPECT_EQ(nullptr, platform.getSystemCompositorController());

        platform.destroyPerRendererComponents();
    }

    TEST_F(APlatformTest, RenderBackendCreationFailsIfSystemCompositorControllerFailedInitialization)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<Platform_BaseMock> platform(rendererConfig);

        createPerRendererComponents(platform, false);
        EXPECT_EQ(nullptr, platform.getSystemCompositorController());

        IRenderBackend* renderBackend = platform.createRenderBackend(displayConfig, windowEventHandlerMock);
        EXPECT_EQ(nullptr, renderBackend);

        platform.destroyPerRendererComponents();
    }
}
