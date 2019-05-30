//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "RendererLib/RendererConfig.h"
#include "WindowEventHandlerMock.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "Platform_Base/PlatformFactory_Base.h"

namespace ramses_internal
{
    namespace
    {
        // If two platform backends are created at the same time, they have to have different id's
        // The numbers have special meaning, therefore the test uses 10000 and 10001 to make sure they
        // don't clash with existing applications
        const WaylandIviSurfaceId iviSurfaceId(10000u);
        const WaylandIviSurfaceId differentIviSurfaceId(10001u);
    }

    using namespace testing;

    class APlatform : public Test
    {
    public:
        APlatform()
        {
            platformFactory = PlatformFactory_Base::CreatePlatformFactory(rendererConfig);
            assert(nullptr != platformFactory);
        }

        ~APlatform()
        {
            delete platformFactory;
        }

    protected:
        IRenderBackend* createRenderBackend(bool multisampling = false, bool useDifferentIviId = false)
        {
            EXPECT_CALL(eventHandlerMock, onFocusChange(_)).Times(AnyNumber());
            EXPECT_CALL(eventHandlerMock, onResize(_, _)).Times(AnyNumber());
            EXPECT_CALL(eventHandlerMock, onMove(_)).Times(AnyNumber());

            DisplayConfig displayConfig;

            if (multisampling)
            {
                displayConfig.setAntialiasingSampleCount(4);
                displayConfig.setAntialiasingMethod(EAntiAliasingMethod_MultiSampling);
            }

            displayConfig.setWaylandIviSurfaceID(useDifferentIviId ? differentIviSurfaceId : iviSurfaceId);
            displayConfig.setWaylandIviLayerID(WaylandIviLayerId(3u));
            // Needed because of compositor - if surface is not turned visible, some of the EGL calls block
            displayConfig.setStartVisibleIvi(true);

            return platformFactory->createRenderBackend(displayConfig, eventHandlerMock);

        }

        RendererConfig rendererConfig;
        IPlatformFactory* platformFactory = nullptr;
        StrictMock<WindowEventHandlerMock>  eventHandlerMock;

    };

    TEST_F(APlatform, CanCreateAndInitializeRenderBackend)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);
        platformFactory->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanCreateMultipleRenderBackends)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IRenderBackend* secondRenderBackend = createRenderBackend(false, true);
        ASSERT_NE(nullptr, secondRenderBackend);

        platformFactory->destroyRenderBackend(*renderBackend);
        platformFactory->destroyRenderBackend(*secondRenderBackend);
    }

    // TODO Violin This does not work on systems which don't have support for multisampling... Needs to be filtered properly
    TEST_F(APlatform, DISABLED_IsEnabledAfterCreationMSAA)
    {
        IRenderBackend* renderBackend = createRenderBackend(true);
        ASSERT_NE(nullptr, renderBackend);
        platformFactory->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanRecreatePlatformFactories)
    {
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platformFactory->destroyRenderBackend(*renderBackend);
        }
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platformFactory->destroyRenderBackend(*renderBackend);
        }
    }

    TEST_F(APlatform, CanBeEnabledMultipleTimes)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().enable());

        platformFactory->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanBeDisabled)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().disable());

        platformFactory->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, Confidence_CanBeEnabledAndDisabledMultipleTimes)
    {

        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().disable());
        EXPECT_TRUE(renderBackend->getSurface().enable());

        platformFactory->destroyRenderBackend(*renderBackend);
    }
}
