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
#include "RendererLib/RenderBackend.h"
#include "RendererLib/ResourceUploadRenderBackend.h"
#include "RendererAPI/ISurface.h"
#include "RendererAPI/IContext.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "Platform_Base/Platform_Base.h"
#include "RendererAPI/IPlatform.h"
#include "Resource/EffectResource.h"
#include <future>
#include <thread>

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
            platform = Platform_Base::CreatePlatform(rendererConfig);
            assert(nullptr != platform);
        }

        ~APlatform()
        {
            delete platform;
        }

    protected:
        IRenderBackend* createRenderBackend(bool multisampling = false, bool useDifferentIviId = false)
        {
            EXPECT_CALL(eventHandlerMock, onResize(_, _)).Times(AnyNumber());
            EXPECT_CALL(eventHandlerMock, onWindowMove(_, _)).Times(AnyNumber());

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

            return platform->createRenderBackend(displayConfig, eventHandlerMock);
        }

        IResourceUploadRenderBackend* createResourceUploadRenderBackend(const IRenderBackend& mainRenderBackend)
        {
            return platform->createResourceUploadRenderBackend(mainRenderBackend);
        }

        std::unique_ptr<const GPUResource> uploadEffectAndExpectSuccess(IDevice& device)
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

            EffectResource effect(vertexShader, fragmentShader, "", {}, {}, "", ResourceCacheFlag_DoNotCache);
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            auto resource = device.uploadShader(effect);
            EXPECT_NE(nullptr, resource);
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            return resource;
        }

        void activateShaderAndExpectSucces(IDevice& device, DeviceResourceHandle deviceHandle)
        {
            EXPECT_TRUE(device.isDeviceStatusHealthy());
            device.activateShader(deviceHandle);
            EXPECT_TRUE(device.isDeviceStatusHealthy());
        }

        RendererConfig rendererConfig;
        IPlatform* platform = nullptr;
        StrictMock<WindowEventHandlerMock>  eventHandlerMock;
    };

    TEST_F(APlatform, CanCreateAndInitializeRenderBackend)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);
        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanCreateMultipleRenderBackends)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IRenderBackend* secondRenderBackend = createRenderBackend(false, true);
        ASSERT_NE(nullptr, secondRenderBackend);

        platform->destroyRenderBackend(*renderBackend);
        platform->destroyRenderBackend(*secondRenderBackend);
    }

    // TODO Violin This does not work on systems which don't have support for multisampling... Needs to be filtered properly
    TEST_F(APlatform, DISABLED_IsEnabledAfterCreationMSAA)
    {
        IRenderBackend* renderBackend = createRenderBackend(true);
        ASSERT_NE(nullptr, renderBackend);
        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanRecreatePlatformFactories)
    {
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platform->destroyRenderBackend(*renderBackend);
        }
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);
            platform->destroyRenderBackend(*renderBackend);
        }
    }

    TEST_F(APlatform, CanBeEnabledMultipleTimes)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().enable());

        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanBeDisabled)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().disable());

        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, Confidence_CanBeEnabledAndDisabledMultipleTimes)
    {

        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(renderBackend->getSurface().disable());
        EXPECT_TRUE(renderBackend->getSurface().enable());

        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanCreateAndInitializeResourceUploadRenderBackend)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
        ASSERT_NE(nullptr, resourceUploadRenderBackend);
        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanCreateResourceUploadRenderBackendsForMultipleRenderBackends)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);

        IRenderBackend* mainRenderBackend2 = createRenderBackend(false, true);
        ASSERT_NE(nullptr, mainRenderBackend2);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
        ASSERT_NE(nullptr, resourceUploadRenderBackend);
        IResourceUploadRenderBackend* resourceUploadRenderBackend2 = createResourceUploadRenderBackend(*mainRenderBackend2);
        ASSERT_NE(nullptr, resourceUploadRenderBackend2);

        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend2);
        platform->destroyRenderBackend(*mainRenderBackend2);

        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanRecreateRenderBackendsWithResourceUploadRenderBackends)
    {
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);

            IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*renderBackend);
            ASSERT_NE(nullptr, resourceUploadRenderBackend);

            platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            platform->destroyRenderBackend(*renderBackend);
        }
        {
            IRenderBackend* renderBackend = createRenderBackend();
            ASSERT_NE(nullptr, renderBackend);

            IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*renderBackend);
            ASSERT_NE(nullptr, resourceUploadRenderBackend);

            platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            platform->destroyRenderBackend(*renderBackend);
        }
    }

    TEST_F(APlatform, ResourceUploadRenderBackendsCanBeEnabledMultipleTimes)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*renderBackend);
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        //try different sequences of alternating between contexts
        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(renderBackend->getSurface().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());

        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, ResourceUploadRenderBackendCanBeDisabled)
    {
        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*renderBackend);
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        EXPECT_TRUE(resourceUploadRenderBackend->getContext().disable());

        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, ResourceUploadRenderBackendCanBeEnabledAndDisabledMultipleTimes)
    {

        IRenderBackend* renderBackend = createRenderBackend();
        ASSERT_NE(nullptr, renderBackend);

        IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*renderBackend);
        ASSERT_NE(nullptr, resourceUploadRenderBackend);

        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().disable());
        EXPECT_TRUE(resourceUploadRenderBackend->getContext().enable());

        platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
        platform->destroyRenderBackend(*renderBackend);
    }

    TEST_F(APlatform, CanCreateAndInitializeResourceUploadRenderBackendInOtherThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<bool> success;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    success.set_value(false);
                    return;
                }

                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
                success.set_value(true);
            });

        EXPECT_TRUE(success.get_future().get());

        resourceUploadThread.join();
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanEnableRenderBackendsInSameTimeInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<bool> successMainThread;
        std::promise<bool> successResourceUploadThread;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    successResourceUploadThread.set_value(false);
                    return;
                }

                const bool enableStatus = resourceUploadRenderBackend->getContext().enable();
                successResourceUploadThread.set_value(enableStatus);

                //block till main context is enabled, to make sure both got enabled succesfully at the same time
                //before the resource upload render backend gets destroyed
                EXPECT_TRUE(successMainThread.get_future().get());
                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            });

        //block till resource upload render backend is created
        EXPECT_TRUE(successResourceUploadThread.get_future().get());
        const bool enableStatus = mainRenderBackend->getSurface().enable();
        EXPECT_TRUE(enableStatus);
        successMainThread.set_value(enableStatus);

        resourceUploadThread.join();
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanUploadResourcesToRenderBackendsInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<bool> success;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    success.set_value(false);
                    return;
                }

                const auto shaderResource = uploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                success.set_value(shaderResource != nullptr);

                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            });

        EXPECT_TRUE(success.get_future().get());

        EXPECT_TRUE(mainRenderBackend->getSurface().enable());
        uploadEffectAndExpectSuccess(mainRenderBackend->getDevice());

        resourceUploadThread.join();
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, EnableContextInMainThreadDoesNotBlockResourceUploadInOtherThread)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<bool> successMainThread;
        std::promise<bool> successCreation;
        std::promise<bool> successResourceUpload;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    successCreation.set_value(false);
                    return;
                }

                //unblock enalbe context in main thread
                successCreation.set_value(true);

                //block till main thread enables context
                EXPECT_TRUE(successMainThread.get_future().get());
                const auto shaderResource = uploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                successResourceUpload.set_value(shaderResource != nullptr);

                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            });

        //block till resource upload thread create resource upload render backend
        EXPECT_TRUE(successCreation.get_future().get());

        //enable main context
        EXPECT_TRUE(mainRenderBackend->getSurface().enable());
        //unblock resource upload in resource upload thread
        successMainThread.set_value(true);

        EXPECT_TRUE(successResourceUpload.get_future().get());

        resourceUploadThread.join();
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanUploadResourceInOneRenderBackendAndUseItInDifferentOneInDifferentThreads)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<std::unique_ptr<const GPUResource>> shaderResource;
        std::promise<bool> resourceUsedSuccess;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    shaderResource.set_value(nullptr);
                    return;
                }

                auto resource = uploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                shaderResource.set_value(std::move(resource));

                //block till resource is used to make sure behavior is deterministic
                EXPECT_TRUE(resourceUsedSuccess.get_future().get());

                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            });


        auto resource = shaderResource.get_future().get();
        EXPECT_NE(resource, nullptr);

        //use resource uploaded in other context/thread
        auto& mainDevice = mainRenderBackend->getDevice();
        const auto deviceHandle = mainDevice.registerShader(std::move(resource));
        EXPECT_TRUE(deviceHandle.isValid());

        EXPECT_TRUE(mainRenderBackend->getSurface().enable());
        activateShaderAndExpectSucces(mainDevice, deviceHandle);
        resourceUsedSuccess.set_value(true);

        resourceUploadThread.join();
        platform->destroyRenderBackend(*mainRenderBackend);
    }

    TEST_F(APlatform, CanUploadResourceInOneRenderBackendAndUseItInDifferentOneInDifferentThreads_AfterResourceUploadRenderBackendDestroyed)
    {
        IRenderBackend* mainRenderBackend = createRenderBackend();
        ASSERT_NE(nullptr, mainRenderBackend);
        EXPECT_TRUE(mainRenderBackend->getSurface().disable());

        std::promise<std::unique_ptr<const GPUResource>> shaderResource;

        std::thread resourceUploadThread([&]()
            {
                IResourceUploadRenderBackend* resourceUploadRenderBackend = createResourceUploadRenderBackend(*mainRenderBackend);
                if (!resourceUploadRenderBackend)
                {
                    shaderResource.set_value(nullptr);
                    return;
                }

                auto resource= uploadEffectAndExpectSuccess(resourceUploadRenderBackend->getDevice());
                shaderResource.set_value(std::move(resource));

                platform->destroyResourceUploadRenderBackend(*resourceUploadRenderBackend);
            });

        resourceUploadThread.join();

        EXPECT_TRUE(mainRenderBackend->getSurface().enable());
        auto resource = shaderResource.get_future().get();
        EXPECT_NE(nullptr, resource);

        //use resource uploaded in other context/thread
        auto& mainDevice = mainRenderBackend->getDevice();
        const auto deviceHandle = mainDevice.registerShader(std::move(resource));
        activateShaderAndExpectSucces(mainDevice, deviceHandle);

        platform->destroyRenderBackend(*mainRenderBackend);
    }
}
