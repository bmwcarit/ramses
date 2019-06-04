//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AbstractSenderAndReceiverTest.h"

#include "ResourceMock.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Components/ManagedResource.h"
#include "Utils/BinaryInputStream.h"
#include "ServiceHandlerMocks.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/BinaryOutputStream.h"
#include "Components/ResourceStreamSerialization.h"
#include "ResourceSerializationTestHelper.h"
#include "Components/SingleResourceSerialization.h"
#include "TestRandom.h"

namespace ramses_internal
{
    using namespace testing;

    class AResourceSenderAndReceiverTest : public AbstractSenderAndReceiverTest
    {
    public:
        AResourceSenderAndReceiverTest()
            : AbstractSenderAndReceiverTest(EServiceType::Ramses)
            , deleterMock(deleterCallbackMock)
        {
            receiver.setResourceConsumerServiceHandler(&handler);
            receiver.setResourceProviderServiceHandler(&providerHandler);
        }

    protected:
        NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
        ResourceDeleterCallingCallback deleterMock;
        StrictMock<ResourceConsumerServiceHandlerMock> handler;
        StrictMock<ResourceProviderServiceHandlerMock> providerHandler;

        template<typename IRESOURCE>
        const IRESOURCE* sendAndReceiveResource(IRESOURCE& resource)
        {
            uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
            uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();
            uint32_t sizeResourcesSent = m_receiverTestWrapper->statisticCollection.statResourcesSentSize.getCounterValue();

            ManagedResource managedRes(resource, deleterMock);
            ResourceInfo resInfo(&resource);

            std::vector<Byte> receivedResourceData;
            {
                PlatformGuard g(receiverExpectCallLock);
                EXPECT_CALL(handler, handleSendResource(_, senderId)).WillOnce(Invoke([this, &receivedResourceData](const ByteArrayView& view, const Guid&)
                {
                    receivedResourceData.insert(receivedResourceData.begin(), view.begin(), view.end());
                    sendEvent();
                }));
            }

            EXPECT_TRUE(sender.sendResources(receiverId, { managedRes }));
            testing::AssertionResult waitRes = waitForEvent();
            EXPECT_TRUE(waitRes);
            if (!waitRes)
            {
                // skip further processing on wait error, cannot use ASSERT_* here
                return NULL;
            }

            ResourceStreamDeserializer deserializer;
            ByteArrayView view(receivedResourceData.data(), static_cast<UInt32>(receivedResourceData.size()));
            std::vector<IResource*> receivedRecources = deserializer.processData(view);
            EXPECT_TRUE(deserializer.processingFinished());
            EXPECT_EQ(1u, receivedRecources.size());

            IRESOURCE* typedResourceToTest = static_cast<IRESOURCE*>(receivedRecources[0]);
            EXPECT_EQ(resInfo.hash, typedResourceToTest->getHash());

            if (resInfo.compressedSize == 0)
            {
                EXPECT_EQ(typedResourceToTest->getDecompressedDataSize(), resInfo.decompressedSize);
            }
            else
            {
                EXPECT_EQ(typedResourceToTest->getCompressedDataSize(), resInfo.compressedSize);
            }

            EXPECT_TRUE(checkResourceDataEqual(resource, *typedResourceToTest));
            EXPECT_EQ(resource.getName(), typedResourceToTest->getName());

            EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
            EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
            EXPECT_LE(sizeResourcesSent + SingleResourceSerialization::SizeOfSerializedResource(resource), m_senderTestWrapper->statisticCollection.statResourcesSentSize.getCounterValue());

            return typedResourceToTest;
        }

        void fillResourceData(IResource& res)
        {
            const uint8_t seed = static_cast<UInt8>(TestRandom::Get(0, 256));
            MemoryBlob& blob = *res.getResourceData();
            for (UInt32 i = 0; i < blob.size(); ++i)
            {
                blob[i] = static_cast<uint8_t>(i+seed);
            }
        }

        bool checkResourceDataEqual(const IResource& resA, const IResource& resB)
        {
            if (!resA.getResourceData() || !resB.getResourceData() ||
                resA.getResourceData()->size() != resB.getResourceData()->size())
            {
                return false;
            }
            return PlatformMemory::Compare(resA.getResourceData()->getRawData(), resB.getResourceData()->getRawData(), resA.getResourceData()->size()) == 0;
        }
    };

    INSTANTIATE_TEST_CASE_P(TypedCommunicationTest, AResourceSenderAndReceiverTest,
                            ::testing::ValuesIn(CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()));

    TEST_P(AResourceSenderAndReceiverTest, ReceivesTheSameTexture2DSentByTheSender)
    {
        const TextureMetaInfo texDesc(16u, 17u, 1u, ETextureFormat_RGBA16, false, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource textureres(EResourceType_Texture2D, texDesc, flag, "resName");
        fillResourceData(textureres);

        const TextureResource* typedResourceToTest = sendAndReceiveResource(textureres);
        ASSERT_TRUE(0 != typedResourceToTest);

        ASSERT_EQ(EResourceType_Texture2D, typedResourceToTest->getTypeID());
        ASSERT_EQ(16u, typedResourceToTest->getWidth());
        ASSERT_EQ(17u, typedResourceToTest->getHeight());
        ASSERT_EQ(1u, typedResourceToTest->getDepth());
        ASSERT_EQ(ETextureFormat_RGBA16, typedResourceToTest->getTextureFormat());
        ASSERT_EQ(texDesc.m_dataSizes, typedResourceToTest->getMipDataSizes());
        ASSERT_EQ(flag, typedResourceToTest->getCacheFlag());
        ASSERT_EQ(texDesc.m_generateMipChain, typedResourceToTest->getGenerateMipChainFlag());
        EXPECT_TRUE(checkResourceDataEqual(textureres, *typedResourceToTest));

        delete typedResourceToTest;
    }

    TEST_P(AResourceSenderAndReceiverTest, ReceivesTheSameTexture3DSentByTheSender)
    {
        const TextureMetaInfo texDesc(16u, 17u, 3u, ETextureFormat_RGBA16, false, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource textureres(EResourceType_Texture3D, texDesc, flag, "resName");
        fillResourceData(textureres);

        const TextureResource* typedResourceToTest = sendAndReceiveResource(textureres);
        ASSERT_TRUE(0 != typedResourceToTest);

        ASSERT_EQ(EResourceType_Texture3D, typedResourceToTest->getTypeID());
        ASSERT_EQ(16u, typedResourceToTest->getWidth());
        ASSERT_EQ(17u, typedResourceToTest->getHeight());
        ASSERT_EQ(3u, typedResourceToTest->getDepth());
        ASSERT_EQ(ETextureFormat_RGBA16, typedResourceToTest->getTextureFormat());
        ASSERT_EQ(texDesc.m_dataSizes, typedResourceToTest->getMipDataSizes());
        ASSERT_EQ(flag, typedResourceToTest->getCacheFlag());
        ASSERT_EQ(texDesc.m_generateMipChain, typedResourceToTest->getGenerateMipChainFlag());
        EXPECT_TRUE(checkResourceDataEqual(textureres, *typedResourceToTest));

        delete typedResourceToTest;
    }

    TEST_P(AResourceSenderAndReceiverTest, ReceivesTheSameTextureCubeSentByTheSender)
    {
        const TextureMetaInfo texDesc(16u, 1u, 1u, ETextureFormat_RGBA16, false, { 1u, 2u });
        const ResourceCacheFlag flag(15u);
        TextureResource textureres(EResourceType_TextureCube, texDesc, flag, "resName");
        fillResourceData(textureres);

        const TextureResource* typedResourceToTest = sendAndReceiveResource(textureres);
        ASSERT_TRUE(0 != typedResourceToTest);

        ASSERT_EQ(EResourceType_TextureCube, typedResourceToTest->getTypeID());
        ASSERT_EQ(16u, typedResourceToTest->getWidth());
        ASSERT_EQ(1u, typedResourceToTest->getHeight());
        ASSERT_EQ(1u, typedResourceToTest->getDepth());
        ASSERT_EQ(ETextureFormat_RGBA16, typedResourceToTest->getTextureFormat());
        ASSERT_EQ(texDesc.m_dataSizes, typedResourceToTest->getMipDataSizes());
        ASSERT_EQ(flag, typedResourceToTest->getCacheFlag());
        ASSERT_EQ(texDesc.m_generateMipChain, typedResourceToTest->getGenerateMipChainFlag());
        EXPECT_TRUE(checkResourceDataEqual(textureres, *typedResourceToTest));

        delete typedResourceToTest;
    }

    TEST_P(AResourceSenderAndReceiverTest, ReceivesTheSameVertexArraySentByTheSender)
    {
        const ResourceCacheFlag flag(15u);
        ArrayResource vertexArrayRes(EResourceType_VertexArray, 3, EDataType_Vector3F, 0u, flag, "resName");
        fillResourceData(vertexArrayRes);

        const ArrayResource* typedResourceToTest = sendAndReceiveResource(vertexArrayRes);
        ASSERT_TRUE(0 != typedResourceToTest);

        ASSERT_EQ(EResourceType_VertexArray, typedResourceToTest->getTypeID());
        ASSERT_EQ(EDataType_Vector3F, typedResourceToTest->getElementType());
        ASSERT_EQ(3u, typedResourceToTest->getElementCount());
        ASSERT_EQ(flag, typedResourceToTest->getCacheFlag());

        delete typedResourceToTest;
    }

    TEST_P(AResourceSenderAndReceiverTest, ReceivesTheSameEffectResourceSentByTheSender)
    {
        String effectName("Some special name");
        const ResourceCacheFlag flag(15u);

        EffectResource effectRes("foo", "bar", EffectInputInformationVector(), EffectInputInformationVector(), effectName, flag);

        const EffectResource* typedResourceToTest = sendAndReceiveResource(effectRes);
        ASSERT_TRUE(0 != typedResourceToTest);

        EXPECT_EQ(effectName, typedResourceToTest->getName());
        EXPECT_STREQ(effectRes.getVertexShader(), typedResourceToTest->getVertexShader());
        EXPECT_STREQ(effectRes.getFragmentShader(), typedResourceToTest->getFragmentShader());
        ASSERT_EQ(flag, typedResourceToTest->getCacheFlag());

        delete typedResourceToTest;
    }

    TEST_P(AResourceSenderAndReceiverTest, SendResourceNotAvailable)
    {
        uint32_t numberMessagesSent = m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue();
        uint32_t numberMessagesReceived = m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue();

        ResourceContentHashVector resourceHashes;
        resourceHashes.push_back(ResourceContentHash(123u, 0));

        ResourceContentHashVector receivedResourceHashes;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(handler, handleResourcesNotAvailable(_, senderId)).WillOnce(DoAll(SaveArg<0>(&receivedResourceHashes), SendHandlerCalledEvent(this)));
        }

        EXPECT_TRUE(sender.sendResourcesNotAvailable(receiverId, resourceHashes));
        ASSERT_TRUE(waitForEvent());

        EXPECT_EQ(resourceHashes, receivedResourceHashes);

        EXPECT_LE(numberMessagesSent + 1, m_senderTestWrapper->statisticCollection.statMessagesSent.getCounterValue());
        EXPECT_LE(numberMessagesReceived + 1, m_receiverTestWrapper->statisticCollection.statMessagesReceived.getCounterValue());
    }

    ACTION_P(AppendArg0ToVector, destVector)
    {
        UNUSED(arg9);
        UNUSED(arg8);
        UNUSED(arg7);
        UNUSED(arg6);
        UNUSED(arg5);
        UNUSED(arg4);
        UNUSED(arg3);
        UNUSED(arg2);
        UNUSED(arg1);
        UNUSED(args);
        destVector->push_back(arg0);
    }

    TEST_P(AResourceSenderAndReceiverTest, ResourceRequestIsSplitInSeveralMessages)
    {
        const CommunicationSendDataSizes sendDataSizes = sender.getSendDataSizes();
        const UInt32 maxNumberOfResourceRequests     = sendDataSizes.resourceInfoNumber;
        const UInt32 expectedNumberOfMessages        = maxNumberOfResourceRequests == std::numeric_limits<UInt32>::max() ? 1u : 2u;
        const UInt32 createdNumberOfResourceRequests = maxNumberOfResourceRequests == std::numeric_limits<UInt32>::max() ? 5u : maxNumberOfResourceRequests+1u;

        ResourceContentHashVector resourceRequestVector;
        resourceRequestVector.reserve(createdNumberOfResourceRequests);
        for(UInt32 i=0; i < createdNumberOfResourceRequests; ++i)
        {
            ResourceContentHash resourceHash(123u + i, 0);
            resourceRequestVector.push_back(resourceHash);
        }

        std::vector<ResourceContentHashVector> receivedResourceRequests;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(providerHandler, handleRequestResources(_, _, senderId)).Times(expectedNumberOfMessages).WillRepeatedly(DoAll(AppendArg0ToVector(&receivedResourceRequests), SendHandlerCalledEvent(this)));
        }

        EXPECT_TRUE(sender.sendRequestResources(receiverId, resourceRequestVector));
        ASSERT_TRUE(waitForEvent(expectedNumberOfMessages));

        ResourceContentHashVector receivedResourceRequestsConcatenated;
        for(const auto& resRequest : receivedResourceRequests)
        {
            receivedResourceRequestsConcatenated.insert(receivedResourceRequestsConcatenated.end(), resRequest.begin(), resRequest.end());
            EXPECT_LE(static_cast<UInt32>(resRequest.size()), maxNumberOfResourceRequests);
        }

        EXPECT_EQ(resourceRequestVector, receivedResourceRequestsConcatenated);
    }

    TEST_P(AResourceSenderAndReceiverTest, sendingResourceVectorResultsInCorrectNumberOfReceiveCallbacks)
    {
        ArrayResource res1(EResourceType_VertexArray, 3, EDataType_Vector3F, 0u, ResourceCacheFlag(0u), "resName");
        fillResourceData(res1);
        ArrayResource res2(EResourceType_VertexArray, 3, EDataType_Vector3F, 0u, ResourceCacheFlag(0u), "resName");
        fillResourceData(res2);

        ManagedResourceVector managedResources = { ManagedResource(res1, deleterMock), ManagedResource(res2, deleterMock) };
        auto sentResourceData = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(managedResources, sender.getSendDataSizes().resourceDataArray);

        std::vector<std::vector<Byte>> receivedResourceData;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(handler, handleSendResource(_, senderId)).Times(1).WillRepeatedly(Invoke([this, &receivedResourceData](const ByteArrayView& resourceData, const Guid&)
            {
                std::vector<Byte> data;
                data.insert(data.begin(), resourceData.begin(), resourceData.end());
                receivedResourceData.push_back(data);
                sendEvent();
            }));
        }

        EXPECT_TRUE(sender.sendResources(receiverId, managedResources));
        ASSERT_TRUE(waitForEvent(static_cast<int>(sentResourceData.size())));

        EXPECT_EQ(sentResourceData, receivedResourceData);
    }

    TEST_P(AResourceSenderAndReceiverTest, sendingResourceVectorWithDifferenttypeOfResourcesResultsInMultipleReceiveCallbacks)
    {
        ArrayResource res1(EResourceType_VertexArray, 3, EDataType_Vector3F, 0u, ResourceCacheFlag(0u), "resName");
        fillResourceData(res1);
        EffectResource res2("foo", "bar", EffectInputInformationVector(), EffectInputInformationVector(), "name", ResourceCacheFlag(0u));
        fillResourceData(res2);

        ManagedResourceVector managedResources = { ManagedResource(res1, deleterMock), ManagedResource(res2, deleterMock) };
        auto sentResourceData = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(managedResources, sender.getSendDataSizes().resourceDataArray);

        std::vector<std::vector<Byte>> receivedResourceData;
        {
            PlatformGuard g(receiverExpectCallLock);
            EXPECT_CALL(handler, handleSendResource(_, senderId)).Times(1).WillRepeatedly(Invoke([this, &receivedResourceData](const ByteArrayView& resourceData, const Guid&)
            {
                std::vector<Byte> data;
                data.insert(data.begin(), resourceData.begin(), resourceData.end());
                receivedResourceData.push_back(data);
                sendEvent();
            }));
        }

        EXPECT_TRUE(sender.sendResources(receiverId, managedResources));
        ASSERT_TRUE(waitForEvent(static_cast<int>(sentResourceData.size())));

        EXPECT_EQ(sentResourceData, receivedResourceData);
    }
}
