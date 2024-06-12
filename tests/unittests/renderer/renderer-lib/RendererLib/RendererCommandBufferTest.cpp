//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RendererCommandBuffer.h"
#include "RendererCommandVisitorMock.h"
#include "internal/Core/Utils/ThreadBarrier.h"
#include <thread>
#include <future>

namespace ramses::internal {
    using namespace testing;

    class ARendererCommandBuffer : public ::testing::Test
    {
    protected:
        void fillBufferWithCommands(RendererCommandBuffer& buffer);
        void expectFilledBufferVisits();

        StrictMock<RendererCommandVisitorMock> visitor{};

        const SceneId sceneId{ 12u };
        const SceneInfo sceneInfo{ sceneId, "testScene" };
        const DisplayHandle displayHandle{ 1u };
        const DisplayConfigData displayConfig{};
        const OffscreenBufferHandle obHandle{ 6u };
        const SceneId providerSceneId{};
        const SceneId consumerSceneId{};
        const DataSlotId providerId{ 1 };
        const DataSlotId consumerId{ 2 };
        const glm::vec4 clearColor{ 1.f, 0.f, 0.5f, 0.2f };
        const StreamBufferHandle streamBuffer{ 7u };
        const WaylandIviSurfaceId source{ 8u };
        const WaylandIviLayerId layer{ 9u };
        IBinaryShaderCache* dummyBinaryShaderCache = reinterpret_cast<IBinaryShaderCache*>(std::uintptr_t{ 0x1 });
    };

    void ARendererCommandBuffer::fillBufferWithCommands(RendererCommandBuffer& buffer)
    {
        buffer.enqueueCommand(RendererCommand::ScenePublished{ sceneId, EScenePublicationMode::LocalAndRemote });
        buffer.enqueueCommand(RendererCommand::SceneUnpublished{ sceneId });
        buffer.enqueueCommand(RendererCommand::ReceiveScene{ sceneInfo });
        buffer.enqueueCommand(RendererCommand::SetSceneState{ sceneId, RendererSceneState::Rendered });
        buffer.enqueueCommand(RendererCommand::SetSceneMapping{ sceneId, displayHandle });
        buffer.enqueueCommand(RendererCommand::SetSceneDisplayBufferAssignment{ sceneId, obHandle, -13 });
        buffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, displayConfig, dummyBinaryShaderCache });
        buffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });
        buffer.enqueueCommand(RendererCommand::LinkData{ providerSceneId, providerId, consumerSceneId, consumerId });
        buffer.enqueueCommand(RendererCommand::UnlinkData{ consumerSceneId, consumerId });
        buffer.enqueueCommand(RendererCommand::SetClearFlags{ displayHandle, obHandle, EClearFlag::Color });
        buffer.enqueueCommand(RendererCommand::SetClearColor{ displayHandle, obHandle, clearColor });
        buffer.enqueueCommand(RendererCommand::SetExterallyOwnedWindowSize{ displayHandle, 1u, 2u});
        buffer.enqueueCommand(RendererCommand::CreateStreamBuffer{ displayHandle, streamBuffer, source });
        buffer.enqueueCommand(RendererCommand::DestroyStreamBuffer{ displayHandle, streamBuffer });
        buffer.enqueueCommand(RendererCommand::LinkStreamBuffer{ streamBuffer, consumerSceneId, consumerId });
        buffer.enqueueCommand(RendererCommand::SCListIviSurfaces{});
        buffer.enqueueCommand(RendererCommand::SCSetIviSurfaceVisibility{ source, true });
        buffer.enqueueCommand(RendererCommand::SCSetIviSurfaceOpacity{ source, 0.5f });
        buffer.enqueueCommand(RendererCommand::SCSetIviLayerVisibility{ layer, true });
        buffer.enqueueCommand(RendererCommand::SCAddIviSurfaceToIviLayer{ source, layer });
        buffer.enqueueCommand(RendererCommand::SCRemoveIviSurfaceFromIviLayer{ source, layer });
        buffer.enqueueCommand(RendererCommand::SCDestroyIviSurface{ source });
    }

    void ARendererCommandBuffer::expectFilledBufferVisits()
    {
        InSequence seq;
        EXPECT_CALL(visitor, handleScenePublished(sceneId, EScenePublicationMode::LocalAndRemote, ERenderBackendCompatibility::OpenGL, EVulkanAPIVersion::Invalid, ESPIRVVersion::Invalid));
        EXPECT_CALL(visitor, handleSceneUnpublished(sceneId));
        EXPECT_CALL(visitor, handleSceneReceived(sceneInfo));
        EXPECT_CALL(visitor, setSceneState(sceneId, RendererSceneState::Rendered));
        EXPECT_CALL(visitor, setSceneMapping(sceneId, displayHandle));
        EXPECT_CALL(visitor, setSceneDisplayBufferAssignment(sceneId, obHandle, -13));
        EXPECT_CALL(visitor, createDisplayContext(displayConfig, displayHandle, dummyBinaryShaderCache));
        EXPECT_CALL(visitor, destroyDisplayContext(displayHandle));
        EXPECT_CALL(visitor, handleSceneDataLinkRequest(providerSceneId, providerId, consumerSceneId, consumerId));
        EXPECT_CALL(visitor, handleDataUnlinkRequest(consumerSceneId, consumerId));
        EXPECT_CALL(visitor, handleSetClearFlags(displayHandle, obHandle, ClearFlags(EClearFlag::Color)));
        EXPECT_CALL(visitor, handleSetClearColor(displayHandle, obHandle, clearColor));
        EXPECT_CALL(visitor, handleSetExternallyOwnedWindowSize(displayHandle, 1u, 2u));
        EXPECT_CALL(visitor, handleBufferCreateRequest(streamBuffer, displayHandle, source));
        EXPECT_CALL(visitor, handleBufferDestroyRequest(streamBuffer, displayHandle));
        EXPECT_CALL(visitor, handleBufferToSceneDataLinkRequest(streamBuffer, consumerSceneId, consumerId));
        EXPECT_CALL(visitor, systemCompositorListIviSurfaces());
        EXPECT_CALL(visitor, systemCompositorSetIviSurfaceVisibility(source, true));
        EXPECT_CALL(visitor, systemCompositorSetIviSurfaceOpacity(source, 0.5f));
        EXPECT_CALL(visitor, systemCompositorSetIviLayerVisibility(layer, true));
        EXPECT_CALL(visitor, systemCompositorAddIviSurfaceToIviLayer(source, layer));
        EXPECT_CALL(visitor, systemCompositorRemoveIviSurfaceFromIviLayer(source, layer));
        EXPECT_CALL(visitor, systemCompositorDestroyIviSurface(source));
    }

    TEST_F(ARendererCommandBuffer, canEnqueueCommands)
    {
        RendererCommandBuffer buffer;
        fillBufferWithCommands(buffer);

        expectFilledBufferVisits();
        visitor.visit(buffer);
    }

    TEST_F(ARendererCommandBuffer, canAddCommandsFromCommandsContainer)
    {
        RendererCommandBuffer buffer;
        fillBufferWithCommands(buffer);
        RendererCommands cmds;
        buffer.swapCommands(cmds);

        RendererCommandBuffer destinationContainer;
        destinationContainer.addAndConsumeCommandsFrom(cmds);

        // expect no visits
        visitor.visit(buffer);

        expectFilledBufferVisits();
        visitor.visit(destinationContainer);
    }

    TEST_F(ARendererCommandBuffer, canAddCommandsFromTwoContainers)
    {
        RendererCommandBuffer destinationContainer;

        RendererCommandBuffer buffer;
        RendererCommands cmds;
        fillBufferWithCommands(buffer);
        buffer.swapCommands(cmds);
        destinationContainer.addAndConsumeCommandsFrom(cmds);

        // expect no visits on consumed buffer
        visitor.visit(buffer);

        fillBufferWithCommands(buffer);
        buffer.swapCommands(cmds);
        destinationContainer.addAndConsumeCommandsFrom(cmds);

        // expect no visits on consumed buffer
        visitor.visit(buffer);

        InSequence seq;
        expectFilledBufferVisits();
        expectFilledBufferVisits();
        visitor.visit(destinationContainer);
    }

    TEST_F(ARendererCommandBuffer, addCommandsFromVariousContainersToContainerInAnotherThread)
    {
        // 2 threads providing commands (e.g. scene control and renderer HL cmd queues)
        // 1 thread consuming those and adding to destContainer (e.g. dispatcher queue)
        // 1 thread consuming destContainer and adding to finalContainer (e.g. executor in displaythread)

        RendererCommandBuffer destContainer;
        RendererCommandBuffer finalContainer;

        ThreadBarrier initBarrier{ 4 };
        ThreadBarrier producersDoneBarrier{ 4 };
        ThreadBarrier consumerProducerDoneBarrier{ 2 };
        ThreadBarrier allDoneBarrier{ 4 };

        std::thread cmdProducer1{ [&]()
        {
            initBarrier.wait();
            RendererCommandBuffer buffer;
            {
                buffer.enqueueCommand(RendererCommand::ScenePublished{ sceneId, EScenePublicationMode::LocalAndRemote });
                buffer.enqueueCommand(RendererCommand::SceneUnpublished{ sceneId });
            }
            RendererCommands cmds;
            buffer.swapCommands(cmds);
            destContainer.addAndConsumeCommandsFrom(cmds);
            producersDoneBarrier.wait();
            allDoneBarrier.wait();
        } };

        std::thread cmdProducer2{ [&]()
        {
            initBarrier.wait();
            RendererCommandBuffer buffer;
            {
                buffer.enqueueCommand(RendererCommand::ReceiveScene{ sceneInfo });
                buffer.enqueueCommand(RendererCommand::SetSceneState{ sceneId, RendererSceneState::Rendered });
            }
            RendererCommands cmds;
            buffer.swapCommands(cmds);
            destContainer.addAndConsumeCommandsFrom(cmds);
            producersDoneBarrier.wait();
            allDoneBarrier.wait();
        } };

        std::thread cmdConsumerAndProducer{ [&]()
        {
            initBarrier.wait();
            RendererCommandBuffer buffer;
            {
                buffer.enqueueCommand(RendererCommand::SetSceneMapping{ sceneId, displayHandle });
                buffer.enqueueCommand(RendererCommand::SetSceneDisplayBufferAssignment{ sceneId, obHandle, -13 });
            }
            RendererCommands cmds;
            buffer.swapCommands(cmds);
            destContainer.addAndConsumeCommandsFrom(cmds);
            producersDoneBarrier.wait();

            destContainer.swapCommands(cmds);
            finalContainer.addAndConsumeCommandsFrom(cmds);
            {
                buffer.enqueueCommand(RendererCommand::CreateDisplay{ displayHandle, displayConfig, dummyBinaryShaderCache });
                buffer.enqueueCommand(RendererCommand::DestroyDisplay{ displayHandle });
            }
            buffer.swapCommands(cmds);
            finalContainer.addAndConsumeCommandsFrom(cmds);
            consumerProducerDoneBarrier.wait();
            allDoneBarrier.wait();
        } };

        std::thread cmdConsumer{ [&]()
        {
            initBarrier.wait();
            producersDoneBarrier.wait();
            consumerProducerDoneBarrier.wait();

            Sequence s1;
            Sequence s2;
            Sequence s3;
            Sequence s4;
            EXPECT_CALL(visitor, handleScenePublished(sceneId, EScenePublicationMode::LocalAndRemote, ERenderBackendCompatibility::OpenGL, EVulkanAPIVersion::Invalid, ESPIRVVersion::Invalid)).InSequence(s1);
            EXPECT_CALL(visitor, handleSceneUnpublished(sceneId)).InSequence(s1);
            EXPECT_CALL(visitor, handleSceneReceived(sceneInfo)).InSequence(s2);
            EXPECT_CALL(visitor, setSceneState(sceneId, RendererSceneState::Rendered)).InSequence(s2);
            EXPECT_CALL(visitor, setSceneMapping(sceneId, displayHandle)).InSequence(s3);
            EXPECT_CALL(visitor, setSceneDisplayBufferAssignment(sceneId, obHandle, -13)).InSequence(s3);
            EXPECT_CALL(visitor, createDisplayContext(displayConfig, displayHandle, dummyBinaryShaderCache)).InSequence(s4);
            EXPECT_CALL(visitor, destroyDisplayContext(displayHandle)).InSequence(s4);
            visitor.visit(finalContainer);

            allDoneBarrier.wait();
        } };

        cmdProducer1.join();
        cmdProducer2.join();
        cmdConsumerAndProducer.join();
        cmdConsumer.join();
    }

    TEST_F(ARendererCommandBuffer, notifiesAnotherThreadAboutNewCommands)
    {
        RendererCommandBuffer buffer;

        ThreadBarrier initialCommandsBarrier{ 2 };
        ThreadBarrier otherCommandsBarrier{ 2 };
        ThreadBarrier doneBarrier{ 2 };

        bool producerCanContinue = false;
        std::mutex producerCanContinueLock;
        std::condition_variable producerCanContinueCvar;

        std::thread cmdProducer{ [&]()
        {
            // some init cmd
            buffer.enqueueCommand(RendererCommand::ScenePublished{ sceneId, EScenePublicationMode::LocalAndRemote });
            initialCommandsBarrier.wait();

            // add another cmd and wait for listener set
            buffer.enqueueCommand(RendererCommand::SceneUnpublished{ sceneId });
            {
                std::unique_lock<std::mutex> lk{ producerCanContinueLock };
                producerCanContinueCvar.wait(lk, [&] { return producerCanContinue; });
            }

            // this cmd is guaranteed to be pushed after consumer waiting using cvar
            buffer.enqueueCommand(RendererCommand::DestroyDisplay{ DisplayHandle{} });
            otherCommandsBarrier.wait();

            buffer.enqueueCommand(RendererCommand::DestroyDisplay{ DisplayHandle{} });
            doneBarrier.wait();
        } };

        std::thread cmdListener{ [&]()
        {
            initialCommandsBarrier.wait();

            {
                std::unique_lock<std::mutex> lk{ producerCanContinueLock };
                producerCanContinue = true;
                producerCanContinueCvar.notify_one();
            }

            // wait forever (20s) till new command comes and do something with it
            RendererCommands cmds;
            buffer.blockingSwapCommands(cmds, std::chrono::milliseconds{20000});
            ASSERT_FALSE(cmds.empty());

            // any other commands will be pushed without notification
            otherCommandsBarrier.wait();

            doneBarrier.wait();
        } };

        cmdProducer.join();
        cmdListener.join();
    }

    TEST_F(ARendererCommandBuffer, canInterruptBlockingSwapCommands)
    {
        RendererCommandBuffer buffer;
        ThreadBarrier initialCommandsBarrier{ 2 };

        std::thread unblocker{ [&]()
        {
            initialCommandsBarrier.wait();
            buffer.interruptBlockingSwapCommands();
        } };

        initialCommandsBarrier.wait();
        RendererCommands cmds;
        buffer.blockingSwapCommands(cmds, std::chrono::milliseconds{1000000000}); // wait forever

        EXPECT_TRUE(cmds.empty());

        unblocker.join();
    }
}
