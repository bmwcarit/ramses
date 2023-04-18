//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SceneUpdateSerializer.h"
#include "TransportCommon/SceneUpdateStreamDeserializer.h"
#include "Components/SceneUpdate.h"
#include "Scene/SceneActionCollection.h"
#include "gtest/gtest.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "ResourceMock.h"
#include "ResourceSerializationTestHelper.h"
#include "Utils/StatisticCollection.h"
#include "gmock/gmock.h"

#include <algorithm>

namespace ramses_internal
{
    class ASceneUpdateSerialization : public ::testing::Test
    {
    public:
        bool serialize(size_t pktSize)
        {
            SceneUpdateSerializer sus(update, sceneStatistics);
            std::vector<Byte> vec(pktSize);
            return sus.writeToPackets({vec.data(), vec.size()}, [&](size_t s) {
                data.push_back(vec);
                data.back().resize(s);
                return true;
            });
        }

        void addTestActions()
        {
            update.actions.beginWriteSceneAction(ESceneActionId::TestAction);
            update.actions.write(static_cast<uint32_t>(123));
            update.actions.write(static_cast<uint32_t>(456));
            update.actions.beginWriteSceneAction(ESceneActionId::AllocateNode);
            update.actions.write(String("foobar"));
        }

        void addFlushInformation()
        {
            update.flushInfos.containsValidInformation = true;
            update.flushInfos.hasSizeInfo = true;
            update.flushInfos.flushCounter = 14;
            update.flushInfos.flushTimeInfo.clock_type = synchronized_clock_type::SystemTime;
            update.flushInfos.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(12345));
            update.flushInfos.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(54321));
            update.flushInfos.resourceChanges.m_resourcesAdded.push_back(ResourceContentHash(77, 66));
            update.flushInfos.resourceChanges.m_resourcesRemoved.push_back(ResourceContentHash(77, 66));
            SceneResourceAction action;
            update.flushInfos.resourceChanges.m_sceneResourceActions.push_back(std::move(action));
            SceneReferenceAction refAction;
            update.flushInfos.sceneReferences.push_back(refAction);
            update.flushInfos.sizeInfo = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
            update.flushInfos.versionTag = SceneVersionTag(2);
        }

        SceneUpdateStreamDeserializer::Result deserialize()
        {
            for (const auto& d : data)
            {
                auto res = deser.processData(d);
                if (res.result != SceneUpdateStreamDeserializer::ResultType::Empty)
                    return res;
            }
            return SceneUpdateStreamDeserializer::Result{SceneUpdateStreamDeserializer::ResultType::Failed, SceneActionCollection(), {}, {}};
        }

        void compare(const SceneUpdateStreamDeserializer::Result& result)
        {
            ASSERT_EQ(SceneUpdateStreamDeserializer::ResultType::HasData, result.result);
            EXPECT_EQ(update.actions, result.actions);
            ASSERT_EQ(update.resources.size(), result.resources.size());
            ASSERT_EQ(update.flushInfos, result.flushInfos);
            for (size_t i = 0; i < update.resources.size(); ++i)
            {
                const IResource* in = update.resources[i].get();
                ASSERT_TRUE(in);
                const IResource* out = result.resources[i].get();
                ResourceSerializationTestHelper::CompareResourceValues(*in, *out);
            }
        }

        ManagedResource CreateTestResource(uint32_t blobSize)
        {
            IResource* res = new ArrayResource(EResourceType_VertexArray, blobSize, EDataType::Vector3F, nullptr, ResourceCacheFlag(15u), "resName");
            if (blobSize)
                ResourceSerializationTestHelper::SetResourceDataRandom(*res, blobSize);
            return ManagedResource{ res, deleterMock };
        }

        void expectDeserializeToSame()
        {
            auto result = deserialize();
            EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::HasData, result.result);
            compare(result);
        }

        SceneUpdateStreamDeserializer deser;
        ResourceDeleterCallingCallback deleterMock;
        SceneUpdate update;
        StatisticCollectionScene sceneStatistics;
        std::vector<std::vector<Byte>> data;
    };

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeEmptyUpdate)
    {
        EXPECT_TRUE(serialize(60));
        EXPECT_EQ(1u, data.size());
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerialieDeserializeSmallSceneActions)
    {
        addTestActions();
        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(1u, data.size());
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerialieDeserializeLargeSceneActions)
    {
        for (size_t i = 0; i < 100; ++i)
            addTestActions();
        EXPECT_TRUE(serialize(100));
        EXPECT_GT(data.size(), 1u);
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, failsSerializeWhenPacketTooSmall)
    {
        EXPECT_FALSE(serialize(49));
        EXPECT_TRUE(serialize(50));
    }

    TEST_F(ASceneUpdateSerialization, canSerialieDeserializeResource)
    {
        update.resources.push_back(CreateTestResource(100));
        EXPECT_TRUE(serialize(100));
        EXPECT_GT(data.size(), 1u);
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerialieDeserializeLargeResource)
    {
        update.resources.push_back(CreateTestResource(100000));
        EXPECT_TRUE(serialize(100));
        EXPECT_GT(data.size(), 1u);
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeFlushInformation)
    {
        addFlushInformation();
        EXPECT_TRUE(serialize(400));
        EXPECT_EQ(1u, data.size());
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, updatesStatisticsForSinglePacket)
    {
        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(1u, sceneStatistics.statSceneUpdatesGeneratedPackets.getCounterValue());
        size_t curSize = data.front().size();
        auto maxSize = curSize;
        data.clear();
        EXPECT_EQ(curSize, sceneStatistics.statSceneUpdatesGeneratedSize.getCounterValue());
        EXPECT_EQ(maxSize, sceneStatistics.statMaximumSizeSingleSceneUpdate.getCounterValue());

        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(2u, sceneStatistics.statSceneUpdatesGeneratedPackets.getCounterValue());
        curSize += data.front().size();
        maxSize = std::max(maxSize, data.front().size());
        data.clear();
        EXPECT_EQ(curSize, sceneStatistics.statSceneUpdatesGeneratedSize.getCounterValue());
        EXPECT_EQ(maxSize, sceneStatistics.statMaximumSizeSingleSceneUpdate.getCounterValue());

        addTestActions();
        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(3u, sceneStatistics.statSceneUpdatesGeneratedPackets.getCounterValue());
        curSize += data.front().size();
        maxSize = std::max(maxSize, data.front().size());
        EXPECT_EQ(curSize, sceneStatistics.statSceneUpdatesGeneratedSize.getCounterValue());
        EXPECT_EQ(maxSize, sceneStatistics.statMaximumSizeSingleSceneUpdate.getCounterValue());
    }

    TEST_F(ASceneUpdateSerialization, updatesStatisticsForMultiplePackets)
    {
        for (size_t i = 0; i < 100; ++i)
            addTestActions();
        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(data.size(), sceneStatistics.statSceneUpdatesGeneratedPackets.getCounterValue());
        size_t overallSize{0};
        for (auto& el: data)
            overallSize += el.size();
        EXPECT_EQ(overallSize, sceneStatistics.statMaximumSizeSingleSceneUpdate.getCounterValue());
    }

    TEST_F(ASceneUpdateSerialization, failsSerializeWhenWriteFunctionFailsOnFirstPacket)
    {
        SceneUpdateSerializer sus(update, sceneStatistics);
        std::vector<Byte> vec(60);
        EXPECT_FALSE(sus.writeToPackets({vec.data(), vec.size()}, [&](size_t) {
            return false;
        }));
    }

    TEST_F(ASceneUpdateSerialization, failsSerializeWhenWriteFunctionFailsOnLaterPacketInResource)
    {
        update.resources.push_back(CreateTestResource(2500));
        SceneUpdateSerializer sus(update, sceneStatistics);
        std::vector<Byte> vec(60);
        int cnt = 0;
        EXPECT_FALSE(sus.writeToPackets({vec.data(), vec.size()}, [&](size_t) {
            if (++cnt == 10)
                return false;
            return true;
        }));
    }

    TEST_F(ASceneUpdateSerialization, failsSerializeWhenWriteFunctionFailsOnLaterPacketInSceneActions)
    {
        for (size_t i = 0; i < 100; ++i)
            addTestActions();
        SceneUpdateSerializer sus(update, sceneStatistics);
        std::vector<Byte> vec(60);
        int cnt = 0;
        EXPECT_FALSE(sus.writeToPackets({vec.data(), vec.size()}, [&](size_t) {
            if (++cnt == 5)
                return false;
            return true;
        }));
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeSceneActionsWithoutData)
    {
        update.actions.beginWriteSceneAction(ESceneActionId::TestAction);
        update.actions.beginWriteSceneAction(ESceneActionId::AllocateNode);
        EXPECT_TRUE(update.actions.collectionData().empty());
        EXPECT_TRUE(serialize(100));
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeResourceWithoutData)
    {
        update.resources.push_back(CreateTestResource(0));
        EXPECT_TRUE(serialize(100));
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, deserializeFailsWhenFirstPacketDroppepd)
    {
        update.resources.push_back(CreateTestResource(100));
        EXPECT_TRUE(serialize(100));
        data.erase(data.begin());

        auto res = deser.processData(data[0]);
        EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Failed, res.result);
    }

    TEST_F(ASceneUpdateSerialization, deserializeFailsWhenLaterPacketDroppepd)
    {
        update.resources.push_back(CreateTestResource(1000));
        EXPECT_TRUE(serialize(100));
        EXPECT_TRUE(data.size() > 10);
        data.erase(data.begin() + 10);
        for (size_t i = 0; i < 10; ++i)
            EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Empty, deser.processData(data[i]).result);
        EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Failed, deser.processData(data[10]).result);
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeComplexUpdate)
    {
        update.resources.push_back(CreateTestResource(100));
        update.resources.push_back(CreateTestResource(200));
        update.resources.push_back(CreateTestResource(10));
        for (size_t i = 0; i < 100; ++i)
            addTestActions();
        EXPECT_TRUE(serialize(100));
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeSameUpdateMultipleTimesWithSameDeserializer)
    {
        update.resources.push_back(CreateTestResource(100));
        update.resources.push_back(CreateTestResource(200));
        update.resources.push_back(CreateTestResource(10));
        for (size_t i = 0; i < 100; ++i)
            addTestActions();
        EXPECT_TRUE(serialize(100));

        expectDeserializeToSame();
        expectDeserializeToSame();
        expectDeserializeToSame();
    }

    TEST_F(ASceneUpdateSerialization, canSerializeDeserializeMultipleTimesWithSameDeserializer)
    {
        {
            update.resources.push_back(CreateTestResource(100));
            update.resources.push_back(CreateTestResource(10));
            for (size_t i = 0; i < 100; ++i)
                addTestActions();

            EXPECT_TRUE(serialize(100));
            expectDeserializeToSame();
        }
        data.clear();
        update = { SceneActionCollection{}, {}, {}};
        {
            update.resources.push_back(CreateTestResource(5));
            update.resources.push_back(CreateTestResource(120));
            update.actions.beginWriteSceneAction(ESceneActionId::AllocateNode);
            update.actions.write(static_cast<uint32_t>(777));
            update.actions.beginWriteSceneAction(ESceneActionId::AllocateNode);
            update.actions.write(static_cast<uint32_t>(666));
            update.actions.beginWriteSceneAction(ESceneActionId::AllocateNode);
            update.actions.write(static_cast<uint32_t>(555));

            EXPECT_TRUE(serialize(100));
            expectDeserializeToSame();
        }
    }

    TEST_F(ASceneUpdateSerialization, failsDeserializeEmptyPacket)
    {
        const auto res = deser.processData({});
        EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Failed, res.result);
    }

    TEST_F(ASceneUpdateSerialization, failsDeserializeZeroFilledPacket)
    {
        const auto res = deser.processData(std::vector<Byte>(60, 0));
        EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Failed, res.result);
    }

    TEST_F(ASceneUpdateSerialization, failsWithTruncatedFirstPacket)
    {
        addTestActions();
        EXPECT_TRUE(serialize(100));
        EXPECT_EQ(1u, data.size());
        for (size_t i = 1; i < 24; ++i)
        {
            SCOPED_TRACE(i);
            std::vector<Byte> vec = data[0];
            vec.resize(i);
            SceneUpdateStreamDeserializer localDeser;
            EXPECT_EQ(SceneUpdateStreamDeserializer::ResultType::Failed, localDeser.processData(vec).result);
        }
    }

    TEST_F(ASceneUpdateSerialization, stressTest)
    {
        std::random_device randomSource;
        unsigned int seed = randomSource();
        SCOPED_TRACE(Message() << "Seed: " << seed);
        std::mt19937 gen(seed);
        auto rnd = [&](uint32_t start, uint32_t end) {
            std::uniform_int_distribution<uint32_t> dis(start, end);
            return dis(gen);
        };
        std::vector<Byte> blob;

        for (int run = 0; run < 500; ++run)
        {
            data.clear();
            update = { SceneActionCollection{}, {}, {} };
            const uint32_t numActions = rnd(0, 200);
            for (uint32_t i = 0; i < numActions; ++i)
            {
                update.actions.beginWriteSceneAction(static_cast<ESceneActionId>(rnd(1, 50)));
                blob.resize(rnd(0, 2000));
                std::iota(blob.begin(), blob.end(), static_cast<Byte>(rnd(0, 255)));
            }

            const uint32_t numResources = rnd(0, 50);
            for (uint32_t i = 0; i < numResources; ++i)
                update.resources.push_back(CreateTestResource(rnd(0, 5000)));

            EXPECT_TRUE(serialize(rnd(50, 5000)));
            expectDeserializeToSame();
        }
    }
}
