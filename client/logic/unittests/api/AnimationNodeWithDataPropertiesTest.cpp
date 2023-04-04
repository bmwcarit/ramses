//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gmock/gmock.h>
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/Property.h"
#include "WithTempDirectory.h"
#include <numeric>

namespace rlogic::internal
{
    // all operations and behavior not specific to node 'with data properties' is tested in AnimationNodeTest.cpp
    class AnAnimationNodeWithDataProperties : public ::testing::Test
    {
    public:
        void SetUp() override
        {
            m_dataFloat1 = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f });
            m_dataFloat2 = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 10.f, 20.f });

            m_configSaveFileWithoutValidation.setValidationEnabled(false);
        }

    protected:
        AnimationNode* createAnimationNodeWithDataProperties(const AnimationChannels& channels, std::string_view name = "")
        {
            AnimationNodeConfig config;
            for (const auto& ch : channels)
            {
                EXPECT_TRUE(config.addChannel(ch));
            }

            if (!config.setExposingOfChannelDataAsProperties(true))
                return nullptr;

            return m_logicEngine.createAnimationNode(config, name);
        }

        static void ExpectArrayDataProperty(const Property& arrayDataProp, const std::vector<float>& arrayValues)
        {
            ASSERT_EQ(EPropertyType::Array, arrayDataProp.getType());
            ASSERT_EQ(arrayValues.size(), arrayDataProp.getChildCount());

            std::vector<float> actualValues;
            for (size_t i = 0u; i < arrayDataProp.getChildCount(); ++i)
            {
                const auto val = arrayDataProp.getChild(i)->get<float>();
                ASSERT_TRUE(val);
                actualValues.push_back(*val);
            }

            EXPECT_EQ(arrayValues, actualValues);
        }

        static void ExpectChannelDataProperties(const Property& channelDataProp, const std::vector<float>& timestamps, const std::vector<float>& keyframes)
        {
            EXPECT_EQ(EPropertyType::Struct, channelDataProp.getType());
            ASSERT_EQ(2u, channelDataProp.getChildCount());

            const auto timestampsProp = channelDataProp.getChild(0u);
            EXPECT_EQ("timestamps", timestampsProp->getName());
            ExpectArrayDataProperty(*timestampsProp, timestamps);

            const auto keyframesProp = channelDataProp.getChild(1u);
            EXPECT_EQ("keyframes", keyframesProp->getName());
            ExpectArrayDataProperty(*keyframesProp, keyframes);
        }

        void advanceAnimationAndExpectValues(AnimationNode& animNode, float progress, float expectedValue)
        {
            EXPECT_TRUE(animNode.getInputs()->getChild("progress")->set(progress));
            EXPECT_TRUE(m_logicEngine.update());
            const auto val = *animNode.getOutputs()->getChild("channel")->get<float>();
            EXPECT_FLOAT_EQ(expectedValue, val);
        }

        LogicEngine m_logicEngine;
        DataArray* m_dataFloat1 = nullptr;
        DataArray* m_dataFloat2 = nullptr;
        SaveFileConfig m_configSaveFileWithoutValidation;
    };

    TEST_F(AnAnimationNodeWithDataProperties, HasAnimationDataProperties)
    {
        const AnimationChannel channel1{ "channel1", m_dataFloat1, m_dataFloat2 };
        const AnimationChannel channel2{ "channel2", m_dataFloat2, m_dataFloat1 };
        const auto animNode = createAnimationNodeWithDataProperties({ channel1, channel2 }, "animNode");

        const auto rootIn = animNode->getInputs();
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(2u, rootIn->getChildCount());
        const auto channelsData = rootIn->getChild(1u);
        EXPECT_EQ("channelsData", channelsData->getName());
        EXPECT_EQ(EPropertyType::Struct, channelsData->getType());

        ASSERT_EQ(2u, channelsData->getChildCount());

        const auto channelData1 = channelsData->getChild(0u);
        EXPECT_EQ("channel1", channelData1->getName());
        ExpectChannelDataProperties(*channelData1, *m_dataFloat1->getData<float>(), *m_dataFloat2->getData<float>());

        const auto channelData2 = channelsData->getChild(1u);
        EXPECT_EQ("channel2", channelData2->getName());
        ExpectChannelDataProperties(*channelData2, *m_dataFloat2->getData<float>(), *m_dataFloat1->getData<float>());
    }

    TEST_F(AnAnimationNodeWithDataProperties, AnimatesAccordinglyWhenKeyframesModified)
    {
        const auto animNode = createAnimationNodeWithDataProperties({ { "channel", m_dataFloat1, m_dataFloat2, EInterpolationType::Linear } });

        // keyframes (0/0, 1/10, 2/20)
        advanceAnimationAndExpectValues(*animNode, 0.0f, 0.f);
        advanceAnimationAndExpectValues(*animNode, 0.05f, 1.f); // time 0.1
        advanceAnimationAndExpectValues(*animNode, 0.25f, 5.f); // time 0.5

        // modify 2nd keyframe -> keyframes (0/0, 1/30, 2/20)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(1u)->set(30.f);
        // will jump to newly interpolated value
        advanceAnimationAndExpectValues(*animNode, 0.25f, 15.f); // time 0.5
        // arrive to 2nd keyframe with new value
        advanceAnimationAndExpectValues(*animNode, 0.5f, 30.f); // time 1.0

        // modify 2nd keyframe again -> keyframes (0/0, 1/100, 2/20)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(1u)->set(100.f);
        // jump to new value right after update, no time change applied
        advanceAnimationAndExpectValues(*animNode, 0.5f, 100.f); // time 1.0

        // modify 1st keyframe -> keyframes (0/-1000, 1/100, 2/20)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(0u)->set(-1000.f);
        // has no effect as we are pass that keyframe
        advanceAnimationAndExpectValues(*animNode, 0.5f, 100.f); // time 1.0

        // modify last keyframe -> keyframes (0/-1000, 1/100, 2/200)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(200.f);
        // has no effect yet we are at 2nd keyframe
        advanceAnimationAndExpectValues(*animNode, 0.5f, 100.f); // time 1.0
        // last keyframe with new value in effect when progressing
        advanceAnimationAndExpectValues(*animNode, 0.75f, 150.f); // time 1.5
        advanceAnimationAndExpectValues(*animNode, 1.f, 200.f); // time 2.0

        // now jump around and test all the new values (0/-1000, 1/100, 2/200)
        advanceAnimationAndExpectValues(*animNode, 0.0f, -1000.f); // time 0.0
        advanceAnimationAndExpectValues(*animNode, 1.0f, 200.f); // time 2.0
        advanceAnimationAndExpectValues(*animNode, 0.25f, -450.f); // time 0.5
        advanceAnimationAndExpectValues(*animNode, 0.75f, 150.f); // time 1.5
        advanceAnimationAndExpectValues(*animNode, 0.5f, 100.f); // time 1.0
    }

    TEST_F(AnAnimationNodeWithDataProperties, AnimatesToNewKeyframeSmoothlyByModifyingBothKeyframesAndTimestamps)
    {
        const auto animNode = createAnimationNodeWithDataProperties({ { "channel", m_dataFloat1, m_dataFloat2, EInterpolationType::Linear } });

        // keyframes (0/0, 1/10, 2/20)
        // for simplicity test will move only between 2 keys in the [1/10, 2/20] range
        advanceAnimationAndExpectValues(*animNode, 0.5f, 10.f); // time 1.0

        // progress towards 20
        advanceAnimationAndExpectValues(*animNode, 0.6f, 12.f); // time 1.2

        // new value target 1000
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(1000.f);
        // to avoid jump which would be caused by lerp between [1/10, 2/1000] at time 1.2 we need to modify previous keyframe as well
        // if we 'move' the previous keyframe to current time and current value, animation will smoothly continue from 'here' to new target
        const float currTime = 1.2f;
        const float currValue = 12.f;
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(1u)->set(currTime);
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(1u)->set(currValue);
        // we modified both previous and next keyframes plus previous timestamp, update with no progress will give same value as before
        advanceAnimationAndExpectValues(*animNode, 0.6f, currValue); // time 1.2
        // now progress to new target 1000
        advanceAnimationAndExpectValues(*animNode, 0.625f, 73.749947f); // time 1.25
        advanceAnimationAndExpectValues(*animNode, 0.65f, 135.49989f); // time 1.3
        advanceAnimationAndExpectValues(*animNode, 0.7f, 259.f); // time 1.4

        // repeat the same again in the middle of ongoing animation -> new value target -1000
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(-1000.f);
        const float currTime2 = 1.4f;
        const float currValue2 = 259.f;
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(1u)->set(currTime2);
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(1u)->set(currValue2);
        advanceAnimationAndExpectValues(*animNode, 0.7f, currValue2); // time 1.4
        // now progress to new target -1000
        advanceAnimationAndExpectValues(*animNode, 0.75f, 49.166626f); // time 1.5
        advanceAnimationAndExpectValues(*animNode, 0.8f, -160.66675f); // time 1.6
        advanceAnimationAndExpectValues(*animNode, 0.9f, -580.3335f); // time 1.8
        advanceAnimationAndExpectValues(*animNode, 1.0f, -1000.f); // time 2.0
    }

    TEST_F(AnAnimationNodeWithDataProperties, ModifyingLastTimestampExtendsWholeAnimation)
    {
        const auto animNode = createAnimationNodeWithDataProperties({ { "channel", m_dataFloat1, m_dataFloat2, EInterpolationType::Linear } });

        // keyframes (0/0, 1/10, 2/20)
        // jump right at the end
        advanceAnimationAndExpectValues(*animNode, 1.0f, 20.f); // time 2

        // modify last timestamp and keyframe to extend animation -> keyframes (0/0, 1/10, 50/1000)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(2u)->set(50.f);
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(1000.f);
        // will jump to newly interpolated value because animation progress is suddenly before its last keyframe
        advanceAnimationAndExpectValues(*animNode, 0.04f, 30.204081f); // time 2
        advanceAnimationAndExpectValues(*animNode, 1.f, 1000.f); // time 50
        EXPECT_FLOAT_EQ(50.f, *animNode->getOutputs()->getChild("duration")->get<float>());

        // extend again -> keyframes (0/0, 1/10, 100/2000)
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(2u)->set(100.f);
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(2000.f);
        // will jump to newly interpolated value because animation progress is suddenly before its last keyframe
        advanceAnimationAndExpectValues(*animNode, 0.5f, 994.94946f); // time 50
        advanceAnimationAndExpectValues(*animNode, 1.f, 2000.f); // time 100
        EXPECT_FLOAT_EQ(100.f, *animNode->getOutputs()->getChild("duration")->get<float>());
    }

    TEST_F(AnAnimationNodeWithDataProperties, ModifyingKeyframeNorTimestampDoesNotAffectChannelDataRetrievedViaGetter)
    {
        const auto animNode = createAnimationNodeWithDataProperties({ { "channel", m_dataFloat1, m_dataFloat2, EInterpolationType::Linear } });

        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(2u)->set(50.f);
        animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(1000.f);
        EXPECT_TRUE(m_logicEngine.update());

        const AnimationChannels& channels = animNode->getChannels();
        ASSERT_EQ(1u, channels.size());
        EXPECT_EQ(m_dataFloat1, channels[0].timeStamps);
        EXPECT_EQ(m_dataFloat2, channels[0].keyframes);
        EXPECT_THAT(*channels[0].timeStamps->getData<float>(), ::testing::ElementsAre(0.f, 1.f, 2.f));
        EXPECT_THAT(*channels[0].keyframes->getData<float>(), ::testing::ElementsAre(0.f, 10.f, 20.f));
    }

    TEST_F(AnAnimationNodeWithDataProperties, CanBeSerializedAndDeserialized)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine otherEngine;

            const auto data1 = otherEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f }, "data1");
            const auto data2 = otherEngine.createDataArray(std::vector<float>{ 0.f, 10.f, 20.f }, "data2");
            const AnimationChannel channel1{ "channel1", data1, data2, EInterpolationType::Linear };
            const AnimationChannel channel2{ "channel2", data2, data1, EInterpolationType::Linear };
            AnimationNodeConfig config;
            ASSERT_TRUE(config.setExposingOfChannelDataAsProperties(true));
            ASSERT_TRUE(config.addChannel(channel1));
            ASSERT_TRUE(config.addChannel(channel2));
            ASSERT_TRUE(otherEngine.createAnimationNode(config, "animNode"));

            ASSERT_TRUE(otherEngine.saveToFile("logic_animNodes.bin", m_configSaveFileWithoutValidation));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_animNodes.bin"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());

        const auto data1 = m_logicEngine.findByName<DataArray>("data1");
        const auto data2 = m_logicEngine.findByName<DataArray>("data2");
        const auto animNode = m_logicEngine.findByName<AnimationNode>("animNode");
        ASSERT_TRUE(data1 && data2 && animNode);

        EXPECT_EQ("animNode", animNode->getName());
        EXPECT_FLOAT_EQ(20.f, *animNode->getOutputs()->getChild("duration")->get<float>());

        const auto rootIn = animNode->getInputs();
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(2u, rootIn->getChildCount());
        const auto channelsData = rootIn->getChild(1u);
        EXPECT_EQ("channelsData", channelsData->getName());
        EXPECT_EQ(EPropertyType::Struct, channelsData->getType());

        ASSERT_EQ(2u, channelsData->getChildCount());

        const auto channelData1 = channelsData->getChild(0u);
        EXPECT_EQ("channel1", channelData1->getName());
        ExpectChannelDataProperties(*channelData1, *data1->getData<float>(), *data2->getData<float>());

        const auto channelData2 = channelsData->getChild(1u);
        EXPECT_EQ("channel2", channelData2->getName());
        ExpectChannelDataProperties(*channelData2, *data2->getData<float>(), *data1->getData<float>());
    }

    TEST_F(AnAnimationNodeWithDataProperties, ResetsChannelDataToOriginalValuesWhenLoadedFromFile)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine otherEngine;

            const auto data1 = otherEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f }, "data1");
            const auto data2 = otherEngine.createDataArray(std::vector<float>{ 0.f, 10.f, 20.f }, "data2");
            const AnimationChannel channel{ "channel", data1, data2, EInterpolationType::Linear };
            AnimationNodeConfig config;
            ASSERT_TRUE(config.setExposingOfChannelDataAsProperties(true));
            ASSERT_TRUE(config.addChannel(channel));
            const auto animNode = otherEngine.createAnimationNode(config, "animNode");
            ASSERT_TRUE(animNode);

            // modify animation data and update
            animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("timestamps")->getChild(2u)->set(50.f);
            animNode->getInputs()->getChild("channelsData")->getChild("channel")->getChild("keyframes")->getChild(2u)->set(1000.f);
            EXPECT_TRUE(otherEngine.update());

            ASSERT_TRUE(otherEngine.saveToFile("logic_animNodes.bin", m_configSaveFileWithoutValidation));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_animNodes.bin"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        EXPECT_TRUE(m_logicEngine.update());

        const auto data1 = m_logicEngine.findByName<DataArray>("data1");
        const auto data2 = m_logicEngine.findByName<DataArray>("data2");
        const auto animNode = m_logicEngine.findByName<AnimationNode>("animNode");
        ASSERT_TRUE(data1 && data2 && animNode);

        EXPECT_EQ("animNode", animNode->getName());
        EXPECT_FLOAT_EQ(2.f, *animNode->getOutputs()->getChild("duration")->get<float>());

        const auto rootIn = animNode->getInputs();
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(2u, rootIn->getChildCount());
        const auto channelsData = rootIn->getChild(1u);
        EXPECT_EQ("channelsData", channelsData->getName());
        EXPECT_EQ(EPropertyType::Struct, channelsData->getType());

        ASSERT_EQ(1u, channelsData->getChildCount());

        const auto channelData = channelsData->getChild(0u);
        EXPECT_EQ("channel", channelData->getName());
        ExpectChannelDataProperties(*channelData, *data1->getData<float>(), *data2->getData<float>());
    }
}
