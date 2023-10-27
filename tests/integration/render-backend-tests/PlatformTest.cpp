//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/RendererLib/RendererConfig.h"
#include "WindowEventHandlerMock.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/RendererLib/RenderBackend.h"
#include "internal/RendererLib/ResourceUploadRenderBackend.h"
#include "internal/RendererLib/PlatformInterface/IContext.h"
#include "internal/RendererLib/PlatformInterface/IDevice.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "internal/Platform/PlatformFactory.h"
#include "internal/RendererLib/PlatformInterface/IPlatform.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Core/Utils/ThreadLocalLog.h"
#include "impl/DisplayConfigImpl.h"
#include "RendererTestUtils.h"
#include <future>
#include <thread>

namespace ramses::internal
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
            PlatformFactory platformFactory;
            auto dispConfig = RendererTestUtils::CreateTestDisplayConfig(0u);
            platform = platformFactory.createPlatform(rendererConfig, dispConfig.impl().getInternalDisplayConfig());
            assert(platform);

            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

    protected:
        IRenderBackend* createRenderBackend(bool multisampling = false, bool useDifferentIviId = false)
        {
            EXPECT_CALL(eventHandlerMock, onResize(_, _)).Times(AnyNumber());
            EXPECT_CALL(eventHandlerMock, onWindowMove(_, _)).Times(AnyNumber());

            DisplayConfig displayConfig;

            if (multisampling)
                displayConfig.setAntialiasingSampleCount(4);

            displayConfig.setWaylandIviSurfaceID(useDifferentIviId ? differentIviSurfaceId : iviSurfaceId);
            displayConfig.setWaylandIviLayerID(WaylandIviLayerId(3u));
            // Needed because of compositor - if surface is not turned visible, some of the EGL calls block
            displayConfig.setStartVisibleIvi(true);

            return platform->createRenderBackend(displayConfig, eventHandlerMock);
        }

        IResourceUploadRenderBackend* createResourceUploadRenderBackend()
        {
            return platform->createResourceUploadRenderBackend();
        }

        static std::unique_ptr<const GPUResource> UploadEffectAndExpectSuccess(IDevice& device)
        {
            const char* vertexShader = R"SHADER(
                    #version 100
                    void main()
                    {
                        gl_Position = vec4(0);
                    }
            )SHADER";

            const char* fragmentShader = R"SHADER(
                    #version 100
                    void main()
                    {
                        gl_FragColor = vec4(0);
                    }
            )SHADER";

            EffectResource effect(vertexShader, fragmentShader, "", {}, {}, {}, "");
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            auto resource = device.uploadShader(effect);
            EXPECT_NE(nullptr, resource);
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            return resource;
        }

        static void ActivateShaderAndExpectSucces(IDevice& device, DeviceResourceHandle deviceHandle)
        {
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            device.activateShader(deviceHandle);
            EXPECT_TRUE(device.isDeviceStatusHealthy());
        }

        RendererConfig rendererConfig;
        std::unique_ptr<IPlatform> platform;
        StrictMock<WindowEventHandlerMock>  eventHandlerMock;
    };

    TEST_F(APlatform, CanCreateAndInitializeRenderBackend)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanRecreatePlatformFactories)
    {
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platform->destroyRenderBackend();
        }
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platform->destroyRenderBackend();
        }
    }

    TEST_F(APlatform, CanBeEnabledMultipleTimes)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getContext().enable());
        EXPECT_TRUE(renderBackend->getContext().enable());
        EXPECT_TRUE(renderBackend->getContext().enable());

        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanBeDisabled)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getContext().disable());

        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, Confidence_CanBeEnabledAndDisabledMultipleTimes)
    {

        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getContext().enable());
        EXPECT_TRUE(renderBackend->getContext().disable());
        EXPECT_TRUE(renderBackend->getContext().enable());

        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanCreateAndInitializeResourceUploadRenderBackend)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
        ASSERT_NE(nullptr, resourceUploadRenderBackend);
        platform->destroyResourceUploadRenderBackend();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanRecreateRenderBackendsWithResourceUploadRenderBackends)
    {
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);

            IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
            ASSERT_NE(nullptr, resourceUploadRenderBackend);

            platform->destroyResourceUploadRenderBackend();
            platform->destroyRenderBackend();
        }
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);

            IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
            ASSERT_NE(nullptr, resourceUploadRenderBackend);

            platform->destroyResourceUploadRenderBackend();
            platform->destroyRenderBackend();
        }
    }

    TEST_F(APlatform, ResourceUploadRenderBackendsCanBeEnabledMultipleTimes)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        //try different sequences of alternating between contexts
        EXPECT_TRUE(renderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(renderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());

        platform->destroyResourceUploadRenderBackend();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, ResourceUploadRenderBackendCanBeDisabled)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        EXPECT_TRUE(resourceUploadRenderBackend->getContext().disable());

        platform->destroyResourceUploadRenderBackend();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, ResourceUploadRenderBackendCanBeEnabledAndDisabledMultipleTimes)
    {

        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().disable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());

        platform->destroyResourceUploadRenderBackend();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanCreateAndInitializeResourceUploadRenderBackendInOtherThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<bool> success;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    success.set_value(false);
                    return;
                }

                platform->destroyResourceUploadRenderBackend();
                success.set_value(true);
            });

        EXPECT_TRUE(success.get_future().get());

        resourceUploadThread.join();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanEnableRenderBackendsInSameTimeInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<bool> successMainThread;
        std::promise<bool> successResourceUploadThread;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    successResourceUploadThread.set_value(false);
                    return;
                }

                const bool enableStatus = resourceUploadRenderBackend->getContext().enable();
                successResourceUploadThread.set_value(enableStatus);

                //block till main context is enabled, to make sure both got enabled successfully at the same time
                //before the resource upload render backend gets destroyed
                EXPECT_TRUE(successMainThread.get_future().get());
                platform->destroyResourceUploadRenderBackend();
            });

        //block till resource upload render backend is created
        EXPECT_TRUE(successResourceUploadThread.get_future().get());
        const bool enableStatus = mainRenderBackend->getContext().enable();
        EXPECT_TRUE(enableStatus);
        successMainThread.set_value(enableStatus);

        resourceUploadThread.join();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanUploadResourcesToRenderBackendsInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<bool> success;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    success.set_value(false);
                    return;
                }

                const auto shaderResource = UploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                success.set_value(shaderResource != nullptr);

                platform->destroyResourceUploadRenderBackend();
            });

        EXPECT_TRUE(success.get_future().get());

        EXPECT_TRUE(mainRenderBackend->getContext().enable());
        UploadEffectAndExpectSuccess(mainRenderBackend->getDevice());

        resourceUploadThread.join();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, EnableContextInMainThreadDoesNotBlockResourceUploadInOtherThread)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<bool> successMainThread;
        std::promise<bool> successCreation;
        std::promise<bool> successResourceUpload;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    successCreation.set_value(false);
                    return;
                }

                //unblock enable context in main thread
                successCreation.set_value(true);

                //block till main thread enables context
                EXPECT_TRUE(successMainThread.get_future().get());
                const auto shaderResource = UploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                successResourceUpload.set_value(shaderResource != nullptr);

                platform->destroyResourceUploadRenderBackend();
            });

        //block till resource upload thread create resource upload render backend
        EXPECT_TRUE(successCreation.get_future().get());

        //enable main context
        EXPECT_TRUE(mainRenderBackend->getContext().enable());
        //unblock resource upload in resource upload thread
        successMainThread.set_value(true);

        EXPECT_TRUE(successResourceUpload.get_future().get());

        resourceUploadThread.join();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanUploadResourceInOneRenderBackendAndUseItInDifferentOneInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<std::unique_ptr<const GPUResource>> shaderResource;
        std::promise<bool> resourceUsedSuccess;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    shaderResource.set_value(nullptr);
                    return;
                }

                auto resource = UploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                shaderResource.set_value(std::move(resource));

                //block till resource is used to make sure behavior is deterministic
                EXPECT_TRUE(resourceUsedSuccess.get_future().get());

                platform->destroyResourceUploadRenderBackend();
            });


        auto resource = shaderResource.get_future().get();
        EXPECT_NE(resource, nullptr);

        //use resource uploaded in other context/thread
        auto& mainDevice = mainRenderBackend->getDevice();
        const auto deviceHandle = mainDevice.registerShader(std::move(resource));
        EXPECT_TRUE(deviceHandle.isValid());

        EXPECT_TRUE(mainRenderBackend->getContext().enable());
        ActivateShaderAndExpectSucces(mainDevice, deviceHandle);
        resourceUsedSuccess.set_value(true);

        resourceUploadThread.join();
        platform->destroyRenderBackend();
    }

    TEST_F(APlatform, CanUploadResourceInOneRenderBackendAndUseItInDifferentOneInDifferentThreads_AfterResourceUploadRenderBackendDestroyed)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getContext().disable());

        std::promise<std::unique_ptr<const GPUResource>> shaderResource;

        std::thread resourceUploadThread([&]()
            {
                // caller is expected to have a display prefix for logs
                ThreadLocalLog::SetPrefix(2);

                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend();
                if (!resourceUploadRenderBackend)
                {
                    shaderResource.set_value(nullptr);
                    return;
                }

                auto resource= UploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                shaderResource.set_value(std::move(resource));

                platform->destroyResourceUploadRenderBackend();
            });

        resourceUploadThread.join();

        EXPECT_TRUE(mainRenderBackend->getContext().enable());
        auto resource = shaderResource.get_future().get();
        EXPECT_NE(nullptr, resource);

        //use resource uploaded in other context/thread
        auto& mainDevice = mainRenderBackend->getDevice();
        const auto deviceHandle = mainDevice.registerShader(std::move(resource));
        ActivateShaderAndExpectSucces(mainDevice, deviceHandle);

        platform->destroyRenderBackend();
    }
}
