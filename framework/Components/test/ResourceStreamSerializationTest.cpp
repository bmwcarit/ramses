//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceStreamSerialization.h"
#include "Components/SingleResourceSerialization.h"
#include "Components/ResourceSerializationHelper.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Resource/ResourceBase.h"
#include "ResourceSerializationTestHelper.h"
#include "gmock/gmock.h"
#include <memory>

namespace ramses_internal
{
    using namespace testing;

    namespace
    {
        using ResourceVector = std::vector<std::unique_ptr<IResource>>;

        class TestResourceStreamSerializer : public ResourceStreamSerializer
        {
        public:
            void serialize(const ManagedResourceVector& resources)
            {
                ResourceStreamSerializer::serialize(
                    [this](UInt32 neededSize) -> std::pair<Byte*, UInt32> {
                        return preparePacket(neededSize);
                    },
                    [this](UInt32 usedSize) {
                        finishedPacket(usedSize);
                    },
                    resources);
            }

            MOCK_METHOD1(preparePacket_cb, UInt32(UInt32));
            MOCK_METHOD1(finishedPacket_cb, void(UInt32));

            std::vector<std::vector<Byte>> packets;

        private:
            std::pair<Byte*, UInt32> preparePacket(UInt32 neededSize)
            {
                const UInt32 maxSize = preparePacket_cb(neededSize);
                packets.push_back(std::vector<Byte>(maxSize, 0));
                return std::make_pair(packets.back().data(), static_cast<UInt32>(packets.back().size()));
            }

            void finishedPacket(UInt32 usedSize)
            {
                assert(!packets.empty());
                assert(usedSize <= packets.back().size());
                packets.back().resize(usedSize);
                finishedPacket_cb(usedSize);
            }
        };

        class TestResource : public ResourceBase
        {
        public:
            TestResource(UInt32 metadataSize, UInt32 blobSize, UInt32 seed,
                ResourceCacheFlag cacheFlag = ResourceCacheFlag(10u), const String& name = String())
                : ResourceBase(EResourceType_Invalid, cacheFlag, name)
            {
                metadata.reserve(metadataSize);
                for (UInt32 i = 0; i < metadataSize; ++i)
                {
                    metadata.push_back(static_cast<Byte>(1 + seed + i * 3));
                }
                SceneResourceData data(new MemoryBlob(blobSize));
                for (UInt32 i = 0; i < data->size(); ++i)
                {
                    (*data)[i] = static_cast<uint8_t>(i + seed);
                }
                setResourceData(data);
            }

            virtual void serializeResourceMetadataToStream(IOutputStream& output) const override
            {
                output << static_cast<UInt32>(metadata.size());
                output.write(metadata.data(), static_cast<UInt32>(metadata.size()));
            }

            static IResource* CreateResourceFromMetadataStream(IInputStream& input, ResourceCacheFlag cacheFlag, const String& name)
            {
                auto res = new TestResource(0, 0, 0, cacheFlag, name);
                UInt32 metadataSize = 0;
                input >> metadataSize;
                res->metadata.resize(metadataSize);
                input.read(reinterpret_cast<Char*>(res->metadata.data()), metadataSize);
                return res;
            }

            std::vector<Byte> metadata;
        };

        void CompareResourceValues(const TestResource& a, const TestResource& b)
        {
            ASSERT_EQ(a.getTypeID(), b.getTypeID());
            EXPECT_EQ(a.getHash(), b.getHash());
            EXPECT_EQ(a.getCacheFlag(), b.getCacheFlag());
            EXPECT_EQ(a.getName(), b.getName());

            EXPECT_EQ(a.metadata, b.metadata);

            ASSERT_TRUE(a.isDeCompressedAvailable());
            ASSERT_TRUE(b.isDeCompressedAvailable());
            ASSERT_TRUE(a.getResourceData().get() != nullptr);
            ASSERT_TRUE(b.getResourceData().get() != nullptr);
            ASSERT_EQ(a.getResourceData()->size(), b.getResourceData()->size());
            EXPECT_EQ(0, PlatformMemory::Compare(a.getResourceData()->getRawData(), b.getResourceData()->getRawData(), a.getResourceData()->size()));
        }
    }

    class AResourceStreamSerialization : public ::testing::Test
    {
    public:
        AResourceStreamSerialization()
            : seed(1)
            , fixedMetadataSize(ResourceSerializationHelper::ResourceMetadataSize(TestResource(0, 0, 0)))
        {
            ResourceSerializationHelper::SetInvalidCreateResourceFromMetadataStreamFunction(&TestResource::CreateResourceFromMetadataStream);
        }

        ~AResourceStreamSerialization()
        {
            ResourceSerializationHelper::SetInvalidCreateResourceFromMetadataStreamFunction(nullptr);
        }

        ManagedResource createTestResource(UInt32 metadataSize, UInt32 blobSize)
        {
            assert(metadataSize >= fixedMetadataSize);
            ManagedResource res(*new TestResource(metadataSize - fixedMetadataSize, blobSize, seed++), deleter);
            assert(ResourceSerializationHelper::ResourceMetadataSize(*res.getResourceObject()) == metadataSize);
            return res;
        }

        ResourceVector deserializeAll()
        {
            std::vector<std::unique_ptr<IResource>> result;
            for (const auto& pkt : serializer.packets)
            {
                ByteArrayView resourceData(pkt.data(), static_cast<UInt32>(pkt.size()));
                std::vector<IResource*> resVec = deserializer.processData(resourceData);
                for (const auto& res : resVec)
                {
                    result.push_back(std::unique_ptr<IResource>(res));
                }
            }
            return result;
        }

        void Compare(const ManagedResourceVector& before, const ResourceVector& after)
        {
            ASSERT_EQ(before.size(), after.size());
            for (UInt i = 0; i < before.size(); ++i)
            {
                SCOPED_TRACE(i);
                ASSERT_TRUE(before[i].getResourceObject() != nullptr);
                ASSERT_TRUE(after[i] != nullptr);
                const IResource& beforeRes = *before[i].getResourceObject();
                const IResource& afterRes = *after[i];
                CompareResourceValues(*beforeRes.convertTo<TestResource>(), *afterRes.convertTo<TestResource>());
            }
        }

        UInt32 seed;
        UInt32 fixedMetadataSize;
        ResourceDeleterCallingCallback deleter;
        StrictMock<TestResourceStreamSerializer> serializer;
        ResourceStreamDeserializer deserializer;
    };

    TEST_F(AResourceStreamSerialization, canSerializeSingleResourceIntoLargerPacket)
    {
        InSequence seq;
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(500));
        EXPECT_CALL(serializer, finishedPacket_cb(_));
        ManagedResourceVector inRes = { createTestResource(40, 50) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, packetWithSingleResourceHasExpectedSize)
    {
        ManagedResource res = createTestResource(40, 50);
        const UInt32 expectedSize = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res.getResourceObject());

        EXPECT_CALL(serializer, preparePacket_cb(expectedSize)).WillOnce(Return(expectedSize));
        EXPECT_CALL(serializer, finishedPacket_cb(expectedSize));

        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
    }

    TEST_F(AResourceStreamSerialization, canSerializeSingleResourceWithoutBlob)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(500));
        EXPECT_CALL(serializer, finishedPacket_cb(_));
        ManagedResourceVector inRes = { createTestResource(40, 0) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeMultipleResourceIntoLargerPacket)
    {
        InSequence seq;
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(2000));
        EXPECT_CALL(serializer, finishedPacket_cb(_));
        ManagedResourceVector inRes = { createTestResource(40, 50), createTestResource(40, 51), createTestResource(40, 52) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, packetWithMultipleResourceHasExpectedSize)
    {
        ManagedResource res1 = createTestResource(40, 50);
        ManagedResource res2 = createTestResource(40, 51);
        ManagedResource res3 = createTestResource(40, 52);

        const UInt32 expectedSize = sizeof(UInt32) + 3 * TestResourceStreamSerializer::FrameSize +
            SingleResourceSerialization::SizeOfSerializedResource(*res1.getResourceObject()) +
            SingleResourceSerialization::SizeOfSerializedResource(*res2.getResourceObject()) +
            SingleResourceSerialization::SizeOfSerializedResource(*res3.getResourceObject());

        EXPECT_CALL(serializer, preparePacket_cb(expectedSize)).WillOnce(Return(expectedSize));
        EXPECT_CALL(serializer, finishedPacket_cb(expectedSize));

        ManagedResourceVector inRes = { res1, res2, res3 };
        serializer.serialize(inRes);
    }

    TEST_F(AResourceStreamSerialization, canSerializeMultipleResourcesWithoutBlob)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(500));
        EXPECT_CALL(serializer, finishedPacket_cb(_));
        ManagedResourceVector inRes = { createTestResource(41, 0), createTestResource(42, 0), createTestResource(43, 0), createTestResource(44, 0) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeMultipleResourcesIntoMultiplePackets)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(50, 100),
            createTestResource(40, 0),
            createTestResource(200, 20000),
            createTestResource(45, 10),
            createTestResource(200, 200) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSpreadSingleLargeResourceOverMultiplePackets)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(250));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(1000, 20000) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeSingleBigResourceFollowedByMultipleSmallOnes)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(200, 20000), createTestResource(40, 0), createTestResource(41, 0), createTestResource(42, 0) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeSingleSmallResourceFollowedByBigOne)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(40, 0), createTestResource(200, 20000) };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeSingleResourceIntoExactlyFittingPacket)
    {
        ManagedResource res = createTestResource(40, 50);
        const UInt32 expectedSize = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res.getResourceObject());
        EXPECT_CALL(serializer, preparePacket_cb(expectedSize)).WillOnce(Return(expectedSize));
        EXPECT_CALL(serializer, finishedPacket_cb(expectedSize));
        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, onlyUsesSizeReallyNeededForResourceEvenIfPacketIsLarger)
    {
        ManagedResource res = createTestResource(40, 50);
        const UInt32 expectedSize = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res.getResourceObject());
        EXPECT_CALL(serializer, preparePacket_cb(expectedSize)).WillOnce(Return(expectedSize+10));
        EXPECT_CALL(serializer, finishedPacket_cb(expectedSize));
        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, usesSecondPacketIfResourceNeedsOneByteMoreThanAvailable)
    {
        ManagedResource res = createTestResource(40, 50);
        const UInt32 expectedSize = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res.getResourceObject());
        const UInt32 sizeOfSecondPacket = sizeof(UInt32) + 1;

        InSequence seq;
        EXPECT_CALL(serializer, preparePacket_cb(expectedSize)).WillOnce(Return(expectedSize - 1));
        EXPECT_CALL(serializer, finishedPacket_cb(expectedSize - 1));
        EXPECT_CALL(serializer, preparePacket_cb(sizeOfSecondPacket)).WillOnce(Return(30));
        EXPECT_CALL(serializer, finishedPacket_cb(sizeOfSecondPacket));

        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, serializeDoesNotFullyUsePacketIfFrameInfoWouldBeSplit)
    {
        ManagedResource res1 = createTestResource(41, 50);
        ManagedResource res2 = createTestResource(40, 0);
        const UInt32 sizeWithoutRes2 = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res1.getResourceObject());
        const UInt32 sizeRes2 = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res2.getResourceObject());

        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(sizeWithoutRes2 + 4)).WillOnce(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(sizeWithoutRes2));
        EXPECT_CALL(serializer, finishedPacket_cb(sizeRes2));

        ManagedResourceVector inRes = { res1, res2 };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, serializeSplitsExactlyAfterFrameInfo)
    {
        ManagedResource res1 = createTestResource(41, 50);
        ManagedResource res2 = createTestResource(40, 0);
        const UInt32 sizeRes1AndFrameOfRes2 = sizeof(UInt32) + 2*TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res1.getResourceObject());
        const UInt32 sizeRestOfRes2 = sizeof(UInt32) + SingleResourceSerialization::SizeOfSerializedResource(*res2.getResourceObject());

        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(sizeRes1AndFrameOfRes2)).WillOnce(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(sizeRes1AndFrameOfRes2));
        EXPECT_CALL(serializer, finishedPacket_cb(sizeRestOfRes2));

        ManagedResourceVector inRes = { res1, res2 };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, splitSerializeInResourceMetadata)
    {
        ManagedResource res = createTestResource(41, 0);
        const UInt32 resSize = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + SingleResourceSerialization::SizeOfSerializedResource(*res.getResourceObject());

        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(resSize - 10)).WillOnce(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(2);

        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, splitSerializeExactlyAfterResourceMetadata)
    {
        ManagedResource res = createTestResource(41, 10);
        const UInt32 resSizeWithoutData = sizeof(UInt32) + TestResourceStreamSerializer::FrameSize + ResourceSerializationHelper::ResourceMetadataSize(*res.getResourceObject());
        const UInt32 resSizeDataOnly = sizeof(UInt32) + 10;

        EXPECT_CALL(serializer, preparePacket_cb(_)).WillOnce(Return(resSizeWithoutData)).WillOnce(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(resSizeWithoutData));
        EXPECT_CALL(serializer, finishedPacket_cb(resSizeDataOnly));

        ManagedResourceVector inRes = { res };
        serializer.serialize(inRes);
        ResourceVector outRes = deserializeAll();
        Compare(inRes, outRes);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, canSerializeTwoConsecutiveIndependantStreams)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(100));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());

        ManagedResourceVector inRes1 = { createTestResource(40, 50), createTestResource(41, 50), createTestResource(42, 50) };
        serializer.serialize(inRes1);
        ResourceVector outRes1 = deserializeAll();
        Compare(inRes1, outRes1);
        EXPECT_TRUE(deserializer.processingFinished());

        serializer.packets.clear();

        ManagedResourceVector inRes2 = { createTestResource(39, 200), createTestResource(38, 50), createTestResource(37, 50) };
        serializer.serialize(inRes2);
        ResourceVector outRes2 = deserializeAll();
        Compare(inRes2, outRes2);
        EXPECT_TRUE(deserializer.processingFinished());
    }

    TEST_F(AResourceStreamSerialization, deserializeFailsOnWrongOrderOfPackets)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(250));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(1000, 20000) };
        serializer.serialize(inRes);

        ASSERT_GT(serializer.packets.size(), 5u);
        std::swap(serializer.packets[3], serializer.packets[5]);

        ResourceVector outRes = deserializeAll();
        EXPECT_TRUE(deserializer.processingFailed());
    }

    TEST_F(AResourceStreamSerialization, deserializeFailsWhenPacketInMiddleMissing)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(250));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(1000, 20000) };
        serializer.serialize(inRes);

        ASSERT_GT(serializer.packets.size(), 5u);
        serializer.packets.erase(serializer.packets.begin() + 3);

        ResourceVector outRes = deserializeAll();
        EXPECT_TRUE(deserializer.processingFailed());
    }

    TEST_F(AResourceStreamSerialization, deserializeFailsWhenFirstPacketMissing)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(250));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        ManagedResourceVector inRes = { createTestResource(1000, 20000) };
        serializer.serialize(inRes);

        ASSERT_GT(serializer.packets.size(), 1u);
        serializer.packets.erase(serializer.packets.begin());

        ResourceVector outRes = deserializeAll();
        EXPECT_TRUE(deserializer.processingFailed());
        EXPECT_TRUE(outRes.empty());
    }

    TEST_F(AResourceStreamSerialization, deserializeFailsWhenLastPacketMissingAndNewStreamIsStarted)
    {
        EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(250));
        EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());

        ManagedResourceVector inRes1 = { createTestResource(1000, 20000) };
        ManagedResourceVector inRes2 = { createTestResource(40, 0) };

        serializer.serialize(inRes1);
        ASSERT_GT(serializer.packets.size(), 5u);
        serializer.packets.pop_back();
        ResourceVector outRes1 = deserializeAll();
        EXPECT_FALSE(deserializer.processingFailed());

        serializer.packets.clear();

        serializer.serialize(inRes2);
        ASSERT_EQ(1u, serializer.packets.size());
        ResourceVector outRes2 = deserializeAll();
        EXPECT_TRUE(deserializer.processingFailed());
        EXPECT_TRUE(outRes2.empty());
    }

    template <typename T>
    class AResourceStreamSerializationTyped : public AResourceStreamSerialization
    {
    public:
        AResourceStreamSerializationTyped()
        {
            EXPECT_CALL(serializer, preparePacket_cb(_)).WillRepeatedly(Return(120));
            EXPECT_CALL(serializer, finishedPacket_cb(_)).Times(AnyNumber());
        }

        ManagedResourceVector createTypedResources(UInt32 num, UInt32 size)
        {
            ManagedResourceVector vec;
            vec.reserve(num);
            for (UInt32 i = 0; i < num; ++i)
            {
                vec.push_back(ManagedResource(*ResourceSerializationTestHelper::CreateTestResource<T>(size), deleter));
            }
            return vec;
        }

        void CompareTyped(const ManagedResourceVector& before, const ResourceVector& after)
        {
            ASSERT_EQ(before.size(), after.size());
            for (UInt i = 0; i < before.size(); ++i)
            {
                const IResource* beforeRes = before[i].getResourceObject();
                const IResource* afterRes = after[i].get();
                ASSERT_TRUE(beforeRes != nullptr);
                ASSERT_TRUE(afterRes != nullptr);
                ResourceSerializationTestHelper::CompareResourceValues(*beforeRes, *afterRes);
                ResourceSerializationTestHelper::CompareTypedResources(static_cast<const T&>(*beforeRes), static_cast<const T&>(*afterRes));
            }
        }
    };

    TYPED_TEST_CASE(AResourceStreamSerializationTyped, ResourceSerializationTestHelper::Types);

    TYPED_TEST(AResourceStreamSerializationTyped, canSerializeDeserializeToSameResources)
    {
        ManagedResourceVector inRes = this->createTypedResources(20, 100);
        this->serializer.serialize(inRes);
        ResourceVector outRes = this->deserializeAll();
        EXPECT_TRUE(this->deserializer.processingFinished());
        this->CompareTyped(inRes, outRes);
    }

    TYPED_TEST(AResourceStreamSerializationTyped, canSerializeCompressedResourceSAndResultsInSameResourceSAfterDecompression)
    {
        ManagedResourceVector inRes = this->createTypedResources(20, 3000); // must be large enough to trigger compression
        for (const auto& res : inRes)
        {
            res.getResourceObject()->compress(IResource::CompressionLevel::REALTIME);
            EXPECT_TRUE(res.getResourceObject()->isCompressedAvailable());
        }

        this->serializer.serialize(inRes);
        ResourceVector outRes = this->deserializeAll();
        EXPECT_TRUE(this->deserializer.processingFinished());

        for (const auto& res : outRes)
        {
            EXPECT_TRUE(res->isCompressedAvailable());
            EXPECT_FALSE(res->isDeCompressedAvailable());
            res->decompress();
            ASSERT_TRUE(res->isDeCompressedAvailable());
        }

        this->CompareTyped(inRes, outRes);
    }
}
