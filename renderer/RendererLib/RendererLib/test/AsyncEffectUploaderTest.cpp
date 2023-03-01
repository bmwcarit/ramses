//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"

#include "RendererLib/AsyncEffectUploader.h"
#include "PlatformMock.h"
#include "Utils/ThreadLocalLog.h"
#include <algorithm>
#include "Watchdog/ThreadAliveNotifierMock.h"
#include <thread>
#include <memory>

using namespace testing;
using namespace std::chrono_literals;
namespace ramses_internal
{
    class AnAsyncEffectUploader : public testing::Test
    {
    protected:
        AnAsyncEffectUploader()
            : asyncEffectUploader(platformMock, platformMock.renderBackendMock, (EXPECT_CALL(notifier, registerThread()).WillOnce(Return(ThreadAliveNotifierMock::dummyThreadId)), notifier), 1)
        {
            // caller of async shader uploader is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(0);
        }

        void TearDown()
        {
            EXPECT_CALL(notifier, unregisterThread(ThreadAliveNotifierMock::dummyThreadId)).Times(1);
        }

        void createResourceUploadingRenderBackend(bool expectNotifications = true)
        {
            {
                InSequence s;
                EXPECT_CALL(platformMock.renderBackendMock.contextMock, disable()).WillOnce(Return(true));
                EXPECT_CALL(platformMock, createResourceUploadRenderBackend());
                EXPECT_CALL(platformMock.renderBackendMock.contextMock, enable()).WillOnce(Return(true));
            }
            if (expectNotifications)
            {
                EXPECT_CALL(notifier, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AnyNumber());
                EXPECT_CALL(notifier, calculateTimeout()).Times(AnyNumber()).WillRepeatedly(Return(10ms));
            }
            const bool status = asyncEffectUploader.createResourceUploadRenderBackendAndStartThread();
            EXPECT_TRUE(status);
        }

        void destroyResourceUploadingRenderBackend()
        {
            EXPECT_CALL(platformMock, destroyResourceUploadRenderBackend());
            asyncEffectUploader.destroyResourceUploadRenderBackendAndStopThread();
            Mock::VerifyAndClearExpectations(&notifier);
        }

        void submitForUploadAndExpectNoShaderWereUploaded(const EffectsRawResources& effectsToUpload)
        {
            EffectsGpuResources uploadedEffects;
            asyncEffectUploader.sync(effectsToUpload, uploadedEffects);

            EXPECT_TRUE(uploadedEffects.empty());
        }

        EffectsRawResources createUniqueEffects(uint32_t count)
        {
            EffectsRawResources result;
            for(uint32_t i = 0u; i < count; ++i)
            {
                const auto randomString = std::to_string(++createdEffectCounter);
                const EffectResource* effect = new EffectResource(randomString.c_str(), "", "", EDrawMode::NUMBER_OF_ELEMENTS, {}, {}, "", ResourceCacheFlag_DoNotCache);
                result.push_back(effect);

                createdEffects.emplace_back(effect); //keep track of created resource to avoid mem-leak
            }

            return result;
        }

        void expectDeviceFlushOnWindows()
        {

#if defined(_WIN32)
            EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, flush()).RetiresOnSaturation();
#endif
        }

        EffectsRawResources createUniqueEffectsAndExpectUploadToDevice(uint32_t count)
        {
            EffectsRawResources result = createUniqueEffects(count);

            for(const auto& effect : result)
                EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(Ref(*effect)));

            expectDeviceFlushOnWindows();

            return result;
        }

        void expectShaderUploadingResult(const EffectsRawResources& effectsToUpload)
        {
            constexpr std::chrono::seconds timeoutTime{ 2u };
            constexpr std::chrono::milliseconds sleepTime {5u};

            const auto startTime = std::chrono::steady_clock::now();
            EffectsGpuResources resultShaders;
            while (resultShaders.empty() && timeoutTime > (std::chrono::steady_clock::now() - startTime))
            {
                asyncEffectUploader.sync({}, resultShaders);
                std::this_thread::sleep_for(sleepTime);
            }

            EXPECT_EQ(resultShaders.size(), effectsToUpload.size());
            EXPECT_TRUE(std::all_of(std::cbegin(effectsToUpload), std::cend(effectsToUpload), [&](const auto& e)
                {
                    return std::find_if(std::cbegin(resultShaders), std::cend(resultShaders), [&e](const auto& u) {return e->getHash() == u.first; }) != resultShaders.cend();
                }));
        }

        void uploadShadersAndExpectSuccess(uint32_t count)
        {
            const EffectsRawResources effectsToUpload = createUniqueEffectsAndExpectUploadToDevice(count);

            //pass effects to upload
            submitForUploadAndExpectNoShaderWereUploaded(effectsToUpload);
            expectShaderUploadingResult(effectsToUpload);
        }

        PlatformStrictMock platformMock;
        StrictMock<ThreadAliveNotifierMock> notifier;
        AsyncEffectUploader asyncEffectUploader;

        std::atomic_uint32_t notifyCounter{ 0 };
        std::atomic_uint32_t timeoutCounter{ 0 };

        std::vector<std::unique_ptr<const EffectResource>> createdEffects; //keep track of created resources to avoid mem-leak
        uint32_t createdEffectCounter = 0u;
    };

    TEST_F(AnAsyncEffectUploader, CanCreateAndDestroyResourceUpoadRenderBackend)
    {
        createResourceUploadingRenderBackend();
        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, FailIfCanNotCreateResourceUpoadRenderBackend)
    {
        InSequence s;
        EXPECT_CALL(platformMock.renderBackendMock.contextMock, disable()).WillOnce(Return(true));
        EXPECT_CALL(platformMock, createResourceUploadRenderBackend()).WillOnce(Return(nullptr));
        EXPECT_CALL(platformMock.renderBackendMock.contextMock, enable()).WillOnce(Return(true));
        const bool status = asyncEffectUploader.createResourceUploadRenderBackendAndStartThread();
        EXPECT_FALSE(status);
    }

    TEST_F(AnAsyncEffectUploader, UploadsSingleShader)
    {
        createResourceUploadingRenderBackend();

        uploadShadersAndExpectSuccess(1u);

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, UploadsSeveralShadersInSequence)
    {
        createResourceUploadingRenderBackend();

        uploadShadersAndExpectSuccess(1u);
        uploadShadersAndExpectSuccess(1u);
        uploadShadersAndExpectSuccess(1u);

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, UploadsSeveralShadersInSameSyncCycle)
    {
        createResourceUploadingRenderBackend();

        uploadShadersAndExpectSuccess(10u);

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, UploadsShaders_ConfidenceTest)
    {
        createResourceUploadingRenderBackend();

        uploadShadersAndExpectSuccess(5u);
        uploadShadersAndExpectSuccess(1u);
        uploadShadersAndExpectSuccess(4u);
        uploadShadersAndExpectSuccess(7u);

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, SyncReturnsEmptyResultIfNoWorkToDo)
    {
        createResourceUploadingRenderBackend();

        uploadShadersAndExpectSuccess(1u);

        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).Times(0u);
        submitForUploadAndExpectNoShaderWereUploaded({});
        submitForUploadAndExpectNoShaderWereUploaded({});
        submitForUploadAndExpectNoShaderWereUploaded({});

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, UploadsShadersThatGetSubmittedForUploadWhileUploadThreadIsBusy)
    {
        createResourceUploadingRenderBackend();

        const auto effectToUploadAndBlock = createUniqueEffects(1u);
        const auto effectsToUploadWhileBusy = createUniqueEffects(6u);

        std::promise<void> barrierRestShaderCanBePushedForUpload;
        std::promise<void> barrierShaderUploadCanBeFinished;

        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(Ref(*effectToUploadAndBlock[0]))).WillOnce(Invoke([&](const auto&) {
            //unblock main thread to allow pushing effects to be uploaded while upload thread is busy
            barrierRestShaderCanBePushedForUpload.set_value();
            //block upload thread till rest of shaders are pushed for upload in calls to sync()
            barrierShaderUploadCanBeFinished.get_future().get();

            return std::make_unique<const GPUResource>(1u, 2u);
            }));
        expectDeviceFlushOnWindows();

        submitForUploadAndExpectNoShaderWereUploaded(effectToUploadAndBlock);
        //block main thread till it is confirmed that upload thread is busy uploading the submitted shader
        barrierRestShaderCanBePushedForUpload.get_future().get();

        //upload rest of shaders, and divide them on several calls to sync
        std::promise<void> barrierRestShadersCanBeUploaded;
        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).Times(5u);
        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).WillOnce(Invoke([&](const auto&) {
            barrierRestShadersCanBeUploaded.get_future().get();
            return std::make_unique<const GPUResource>(1u, 2u);
            })).RetiresOnSaturation();
        expectDeviceFlushOnWindows();

        submitForUploadAndExpectNoShaderWereUploaded({ effectsToUploadWhileBusy[0], effectsToUploadWhileBusy[1] });
        submitForUploadAndExpectNoShaderWereUploaded({ effectsToUploadWhileBusy[2], effectsToUploadWhileBusy[3] });
        submitForUploadAndExpectNoShaderWereUploaded({ effectsToUploadWhileBusy[4], effectsToUploadWhileBusy[5] });

        //unblock upload thread for 1st ever effect
        barrierShaderUploadCanBeFinished.set_value();
        expectShaderUploadingResult(effectToUploadAndBlock);
        //unblock upload thread for effects that got submitted while upload thread is busy
        barrierRestShadersCanBeUploaded.set_value();
        expectShaderUploadingResult(effectsToUploadWhileBusy);

        destroyResourceUploadingRenderBackend();
    }

    TEST_F(AnAsyncEffectUploader, notifiesWatchdogInbetweenEveryShaderUpload)
    {
        EXPECT_CALL(notifier, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(5)).WillRepeatedly([this](auto) { notifyCounter++; });
        EXPECT_CALL(notifier, calculateTimeout()).Times(AtLeast(0)).WillRepeatedly([this]() { timeoutCounter++; return 20ms; });
        createResourceUploadingRenderBackend(false);

        uploadShadersAndExpectSuccess(5u);

        destroyResourceUploadingRenderBackend();
        EXPECT_EQ(notifyCounter, timeoutCounter + 5u);
    }

    TEST_F(AnAsyncEffectUploader, notifiesWatchdogRegularlyWithNoWorkToDo)
    {
        EXPECT_CALL(notifier, notifyAlive(ThreadAliveNotifierMock::dummyThreadId)).Times(AtLeast(10)).WillRepeatedly([this](auto) { notifyCounter++; });
        EXPECT_CALL(notifier, calculateTimeout()).Times(AtLeast(10)).WillRepeatedly([this]() { timeoutCounter++; return 1ms; });
        createResourceUploadingRenderBackend(false);

        while (timeoutCounter < 10)
            std::this_thread::sleep_for(1ms);

        destroyResourceUploadingRenderBackend();
        EXPECT_EQ(notifyCounter, timeoutCounter);
    }

    TEST_F(AnAsyncEffectUploader, DestructionCancelsUpcomingShaderUploads)
    {
        createResourceUploadingRenderBackend();
        constexpr uint32_t effectCount = 10u;
        const auto effects = createUniqueEffects(effectCount);

        constexpr std::chrono::milliseconds nonTrivialTime{ 100u };
        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(_)).Times(AnyNumber()).WillRepeatedly(Invoke([&](const auto&) {
                std::this_thread::sleep_for(nonTrivialTime);
                return std::make_unique<const GPUResource>(1u, 2u);
            }));

        //destroy can be called only AFTER shader upload has started, otherwise the whole logic of shader upload might not be triggered
        //so test needs to block main thread till 1st shader upload
        std::promise<void> barrierDestroyCanBeCalled;
        const auto& effectToUploadAndBlock = effects[0];
        EXPECT_CALL(platformMock.resourceUploadRenderBackendMock.deviceMock, uploadShader(Ref(*effectToUploadAndBlock))).WillOnce(Invoke([&](const auto&) {
            //unblock main thread to allow destroy to be called while thread is busy with shader upload
            barrierDestroyCanBeCalled.set_value();

            return std::make_unique<const GPUResource>(1u, 2u);
            })).RetiresOnSaturation();
        expectDeviceFlushOnWindows();

        submitForUploadAndExpectNoShaderWereUploaded(effects);

        //block main thread till it is confirmed that upload thread is busy uploading the submitted shader
        barrierDestroyCanBeCalled.get_future().get();

        const auto timeBeforeDestroyCall = std::chrono::steady_clock::now();
        destroyResourceUploadingRenderBackend();
        const auto durationDestroyCallBlocked = std::chrono::steady_clock::now() - timeBeforeDestroyCall;

        const auto maxDurationShaderUpload = nonTrivialTime * (effectCount - 1u);
        EXPECT_TRUE(durationDestroyCallBlocked < maxDurationShaderUpload);
    }
}
