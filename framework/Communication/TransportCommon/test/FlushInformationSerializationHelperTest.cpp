//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SceneUpdateSerializationHelper.h"
#include "Components/SceneUpdate.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class AFlushInformationSerialization : public ::testing::Test
    {
    public:
        FlushInformation SerializeDeserialize(const FlushInformation& flushInfos)
        {
            const absl::Span<const Byte> infos = FlushInformationSerialization::SerializeInfos(flushInfos, workingMem);
            return FlushInformationSerialization::Deserialize(infos);
        }

        std::vector<Byte> workingMem;
        FlushInformation in{};
    };

    TEST_F(AFlushInformationSerialization, minimumSizeIsSizeOfEmptyFlushInfoSerialized)
    {
        const absl::Span<const Byte> infos = FlushInformationSerialization::SerializeInfos(in, workingMem);
        ASSERT_EQ(FlushInformation::getMinimumSize(), infos.size());
    }

    TEST_F(AFlushInformationSerialization, canSerializeDeserializeDefaultFlushInformation)
    {
        EXPECT_EQ(in, SerializeDeserialize(in));
    }

    TEST_F(AFlushInformationSerialization, canSerializeDeserializeFlushInformationWithData)
    {
        in.containsValidInformation = true;
        in.hasSizeInfo = true;
        in.flushCounter = 14;
        in.flushTimeInfo.clock_type = synchronized_clock_type::SystemTime;
        in.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(12345));
        in.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(54321));
        in.resourceChanges.m_addedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        in.resourceChanges.m_removedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        SceneResourceAction action;
        in.resourceChanges.m_sceneResourceActions.push_back(std::move(action));
        SceneReferenceAction refAction{ SceneReferenceActionType::LinkData, SceneReferenceHandle{ 1 }, DataSlotId{ 1 }, SceneReferenceHandle{ 2 }, DataSlotId{ 2 } };
        in.sceneReferences.push_back(refAction);
        in.sizeInfo = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
        in.versionTag = SceneVersionTag(2);
        EXPECT_EQ(in, SerializeDeserialize(in));
    }

    TEST_F(AFlushInformationSerialization, canSerializeDeserializeFlushInformationWithoutSizeInfoAndWithoutSceneReferences)
    {
        in.containsValidInformation = true;
        in.hasSizeInfo = false;
        in.flushCounter = 14;
        in.flushTimeInfo.clock_type = synchronized_clock_type::PTP;
        in.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(12345));
        in.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(54321));
        in.resourceChanges.m_addedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        in.resourceChanges.m_removedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        SceneResourceAction action;
        in.resourceChanges.m_sceneResourceActions.push_back(std::move(action));
        in.versionTag = SceneVersionTag(2);
        EXPECT_EQ(in, SerializeDeserialize(in));
    }

    TEST_F(AFlushInformationSerialization, canPrintFlushInformation)
    {
        in.containsValidInformation = true;
        in.hasSizeInfo = true;
        in.flushCounter = 14;
        in.flushTimeInfo.clock_type = synchronized_clock_type::SystemTime;
        in.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(12345));
        in.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(54321));
        in.resourceChanges.m_addedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        in.resourceChanges.m_removedClientResourceRefs.push_back(ResourceContentHash(77, 66));
        SceneResourceAction action;
        in.resourceChanges.m_sceneResourceActions.push_back(std::move(action));
        SceneReferenceAction refAction{ SceneReferenceActionType::LinkData, SceneReferenceHandle{ 1 }, DataSlotId{ 1 }, SceneReferenceHandle{ 2 }, DataSlotId{ 2 } };
        in.sceneReferences.push_back(refAction);
        in.sizeInfo = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
        in.versionTag = SceneVersionTag(2);

        EXPECT_EQ(fmt::to_string(in),
            "FlushInformation:[valid:true;flushcounter:14;version:2;resChanges[+:1;-:1;resActions:1];refActions:1;time[0;exp:12345;int:54321];sizeInfo:[ node=1 camera=2 transform=3 renderable=4 state=5 datalayout=6 datainstance=7 renderGroup=8 renderPass=9 blitPass=10 renderTarget=11 renderBuffer=12 textureSampler=13 streamTexture=14 dataSlot=15 dataBuffer=16 animationSystem=17 textureBuffer=18 pickableObjectCount=19 sceneReferenceCount=20 ]]");
    }

}
