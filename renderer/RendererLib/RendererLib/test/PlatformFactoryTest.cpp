//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "PlatformFactory_BaseMock.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "WindowEventHandlerMock.h"

using namespace testing;

namespace ramses_internal
{
    class APlatformFactoryTest : public Test
    {
    protected:
        IRenderBackend* createRenderBackend(PlatformFactory_BaseMock& platformFactory)
        {
            platformFactory.createRenderBackendMockObjects();

            InSequence s;
            EXPECT_CALL(platformFactory, createWindow(_, _));
            EXPECT_CALL(*platformFactory.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createContext(Ref(*platformFactory.window)));
            EXPECT_CALL(*platformFactory.context, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createSurface(Ref(*platformFactory.window), Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.surface, enable()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createDevice(Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.device, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createEmbeddedCompositor());
            EXPECT_CALL(*platformFactory.embeddedCompositor, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createTextureUploadingAdapter(Ref(*platformFactory.device), Ref(*platformFactory.embeddedCompositor), Ref(*platformFactory.window)));

            return platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
        }

        void createPerRendererComponents(PlatformFactory_BaseMock& platformFactory, Bool initSystemCompositorControllerSucess = true)
        {
            platformFactory.systemCompositorController = platformFactory.createMockObjectHelper<SystemCompositorControllerMockWithDestructor>();

            InSequence s;
            EXPECT_CALL(platformFactory, createSystemCompositorController());
            EXPECT_CALL(*platformFactory.systemCompositorController, init()).WillOnce(Return(initSystemCompositorControllerSucess));

            if(initSystemCompositorControllerSucess)
            {
                platformFactory.createPerRendererComponents();

            }
            else
            {
                EXPECT_CALL(static_cast<SystemCompositorControllerMockWithDestructor&>(*platformFactory.systemCompositorController), Die()); //expect destructor called

                platformFactory.createPerRendererComponents();

                Mock::VerifyAndClearExpectations(platformFactory.systemCompositorController);
                platformFactory.systemCompositorController = nullptr;
            }
        }

        void verifyAndClearExpectationsOnRenderBackendMockObjects(PlatformFactory_BaseMock& platformFactory)
        {
            Mock::VerifyAndClearExpectations(platformFactory.window);
            Mock::VerifyAndClearExpectations(platformFactory.context);
            Mock::VerifyAndClearExpectations(platformFactory.surface);
            Mock::VerifyAndClearExpectations(platformFactory.device);
            Mock::VerifyAndClearExpectations(platformFactory.embeddedCompositor);
            Mock::VerifyAndClearExpectations(platformFactory.textureUploadingAdapter);
        }

        void destroyRenderBackend(PlatformFactory_BaseMock& platformFactory, IRenderBackend& renderBackend)
        {
            InSequence s;
            EXPECT_CALL(*platformFactory.textureUploadingAdapter, Die());
            EXPECT_CALL(*platformFactory.embeddedCompositor, Die());
            EXPECT_CALL(*platformFactory.device, Die());
            EXPECT_CALL(*platformFactory.surface, Die());
            EXPECT_CALL(*platformFactory.context, Die());
            EXPECT_CALL(*platformFactory.window, Die());

            platformFactory.destroyRenderBackend(renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platformFactory);

            platformFactory.window                  = nullptr;
            platformFactory.context                 = nullptr;
            platformFactory.surface                 = nullptr;
            platformFactory.device                  = nullptr;
            platformFactory.embeddedCompositor      = nullptr;
            platformFactory.textureUploadingAdapter = nullptr;
        }

        void destroyPerRendererComponents(PlatformFactory_BaseMock& platformFactory)
        {
            EXPECT_CALL(static_cast<SystemCompositorControllerMockWithDestructor&>(*platformFactory.systemCompositorController), Die());
            platformFactory.destroyPerRendererComponents();

            Mock::VerifyAndClearExpectations(platformFactory.systemCompositorController);

            platformFactory.systemCompositorController = nullptr;
        }

        RendererConfig                  rendererConfig;
        DisplayConfig                   displayConfig;
        WindowEventHandlerMock          windowEventHandlerMock;
    };

    TEST_F(APlatformFactoryTest, CanCreateAndDestroyRenderBackend)
    {
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);
        IRenderBackend* renderBackend = createRenderBackend(platformFactory);
        ASSERT_TRUE(NULL != renderBackend);

        destroyRenderBackend(platformFactory, *renderBackend);
    }

    TEST_F(APlatformFactoryTest, RenderBackendCreationFailsIfWindowFailsInitialization)
    {
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platformFactory, createWindow(_, _));
            EXPECT_CALL(*platformFactory.window, init()).WillOnce(Return(false)); //window fails init
            //destructor gets called
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platformFactory.window), Die());

            IRenderBackend* renderBackend = platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platformFactory);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<TextureUploadingAdapterMockWithDestructor&>(*platformFactory.textureUploadingAdapter), Die());
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platformFactory.embeddedCompositor), Die());
        EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platformFactory.device), Die());
        EXPECT_CALL(static_cast<SurfaceMockWithDestructor&>(*platformFactory.surface), Die());
        EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platformFactory.context), Die());
        delete platformFactory.context;
        delete platformFactory.surface;
        delete platformFactory.device;
        delete platformFactory.embeddedCompositor;
        delete platformFactory.textureUploadingAdapter;
    }

    TEST_F(APlatformFactoryTest, RenderBackendCreationFailsIfContextFailsInitialization)
    {
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platformFactory, createWindow(_, _));
            EXPECT_CALL(*platformFactory.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createContext(Ref(*platformFactory.window)));
            EXPECT_CALL(*platformFactory.context, init()).WillOnce(Return(false)); //context fails init
            //destructors gets called
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platformFactory.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platformFactory.window), Die());

            IRenderBackend* renderBackend = platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platformFactory);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<TextureUploadingAdapterMockWithDestructor&>(*platformFactory.textureUploadingAdapter), Die());
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platformFactory.embeddedCompositor), Die());
        EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platformFactory.device), Die());
        EXPECT_CALL(static_cast<SurfaceMockWithDestructor&>(*platformFactory.surface), Die());
        delete platformFactory.surface;
        delete platformFactory.device;
        delete platformFactory.embeddedCompositor;
        delete platformFactory.textureUploadingAdapter;
    }

    TEST_F(APlatformFactoryTest, RenderBackendCreationFailsIfDeviceFailsInitialization)
    {
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platformFactory, createWindow(_, _));
            EXPECT_CALL(*platformFactory.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createContext(Ref(*platformFactory.window)));
            EXPECT_CALL(*platformFactory.context, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createSurface(Ref(*platformFactory.window), Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.surface, enable()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createDevice(Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.device, init()).WillOnce(Return(false)); //device fails init

            //destructors gets called
            EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platformFactory.device), Die());
            EXPECT_CALL(static_cast<SurfaceMockWithDestructor&>(*platformFactory.surface), Die());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platformFactory.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platformFactory.window), Die());

            IRenderBackend* renderBackend = platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platformFactory);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<TextureUploadingAdapterMockWithDestructor&>(*platformFactory.textureUploadingAdapter), Die());
        EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platformFactory.embeddedCompositor), Die());
        delete platformFactory.embeddedCompositor;
        delete platformFactory.textureUploadingAdapter;
    }


    TEST_F(APlatformFactoryTest, RenderBackendCreationFailsIfEmbeddedCompositorFailsInitialization)
    {
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createRenderBackendMockObjects();

        {
            InSequence s;
            EXPECT_CALL(platformFactory, createWindow(_, _));
            EXPECT_CALL(*platformFactory.window, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createContext(Ref(*platformFactory.window)));
            EXPECT_CALL(*platformFactory.context, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createSurface(Ref(*platformFactory.window), Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.surface, enable()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createDevice(Ref(*platformFactory.context)));
            EXPECT_CALL(*platformFactory.device, init()).WillOnce(Return(true));

            EXPECT_CALL(platformFactory, createEmbeddedCompositor());
            EXPECT_CALL(*platformFactory.embeddedCompositor, init()).WillOnce(Return(false)); //embedded compositor fails init

            //destructors gets called
            EXPECT_CALL(static_cast<EmbeddedCompositorMockWithDestructor&>(*platformFactory.embeddedCompositor), Die());
            EXPECT_CALL(static_cast<DeviceMockWithDestructor&>(*platformFactory.device), Die());
            EXPECT_CALL(static_cast<SurfaceMockWithDestructor&>(*platformFactory.surface), Die());
            EXPECT_CALL(static_cast<ContextMockWithDestructor&>(*platformFactory.context), Die());
            EXPECT_CALL(static_cast<WindowMockWithDestructor&>(*platformFactory.window), Die());

            IRenderBackend* renderBackend = platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
            EXPECT_EQ(nullptr, renderBackend);

            verifyAndClearExpectationsOnRenderBackendMockObjects(platformFactory);
        }

        //delete the rest of un-used mock objects
        EXPECT_CALL(static_cast<TextureUploadingAdapterMockWithDestructor&>(*platformFactory.textureUploadingAdapter), Die());
        delete platformFactory.textureUploadingAdapter;
    }

    TEST_F(APlatformFactoryTest, CanCreateAndDestroyPerRendererComponents_WithoutSystemCompositorController)
    {
        ASSERT_FALSE(rendererConfig.getSystemCompositorControlEnabled());
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createPerRendererComponents();

        EXPECT_EQ(nullptr, platformFactory.getSystemCompositorController());
        platformFactory.destroyPerRendererComponents();
    }

    TEST_F(APlatformFactoryTest, CanCreateRenderBackendIfSystemCompositorControllerIsNotEnabled)
    {
        ASSERT_FALSE(rendererConfig.getSystemCompositorControlEnabled());
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        platformFactory.createPerRendererComponents();

        EXPECT_EQ(nullptr, platformFactory.getSystemCompositorController());

        IRenderBackend* renderBackend = createRenderBackend(platformFactory);
        EXPECT_NE(nullptr, renderBackend);
        destroyRenderBackend(platformFactory, *renderBackend);

        platformFactory.destroyPerRendererComponents();
    }

    TEST_F(APlatformFactoryTest, CanCreateAndDestroyPerRendererComponents_WithSystemCompositorController)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        createPerRendererComponents(platformFactory);

        EXPECT_NE(nullptr, platformFactory.getSystemCompositorController());
        EXPECT_EQ(platformFactory.systemCompositorController, platformFactory.getSystemCompositorController());
        destroyPerRendererComponents(platformFactory);
    }

    TEST_F(APlatformFactoryTest, DestroysSystemCompositorIfItFailsInitialization)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        createPerRendererComponents(platformFactory, false);
        EXPECT_EQ(nullptr, platformFactory.getSystemCompositorController());

        platformFactory.destroyPerRendererComponents();
    }

    TEST_F(APlatformFactoryTest, RenderBackendCreationFailsIfSystemCompositorControllerFailedInitialization)
    {
        rendererConfig.enableSystemCompositorControl();
        StrictMock<PlatformFactory_BaseMock> platformFactory(rendererConfig);

        createPerRendererComponents(platformFactory, false);
        EXPECT_EQ(nullptr, platformFactory.getSystemCompositorController());

        IRenderBackend* renderBackend = platformFactory.createRenderBackend(displayConfig, windowEventHandlerMock);
        EXPECT_EQ(nullptr, renderBackend);

        platformFactory.destroyPerRendererComponents();
    }
}
