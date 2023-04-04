//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "WithTempDirectory.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/Property.h"
#include "impl/AnimationNodeImpl.h"
#include "impl/DataArrayImpl.h"
#include "impl/PropertyImpl.h"
#include "internals/ErrorReporting.h"
#include "internals/SerializationMap.h"
#include "internals/DeserializationMap.h"
#include "internals/TypeData.h"
#include "internals/EPropertySemantics.h"
#include "generated/AnimationNodeGen.h"
#include "flatbuffers/flatbuffers.h"
#include <numeric>

namespace rlogic::internal
{
    class AnAnimationNode : public ::testing::TestWithParam<bool>
    {
    public:
        void SetUp() override
        {
            m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
            m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
            // Quaternions which are not normalized (ie. not of unit length). Used for tests to check they are normalized correctly
            m_dataVec4 = m_logicEngine.createDataArray(std::vector<vec4f>{ { 2.f, 0.f, 0.f, 0.f }, { 0.f, 2.f, 0.f, 0.f } , { 0.f, 0.f, 2.f, 0.f } });
            m_dataVecVec = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 1.f, 2.f, 3.f, 4.f, 5.f }, { 3.f, 4.f, 5.f, 6.f, 7.f }, { 5.f, 6.f, 7.f, 8.f, 9.f } });

            m_saveFileConfigNoValidation.setValidationEnabled(false);
        }

    protected:
        AnimationNode* createAnimationNode(const AnimationChannels& channels, std::string_view name = "")
        {
            AnimationNodeConfig config;
            for (const auto& ch : channels)
            {
                EXPECT_TRUE(config.addChannel(ch));
            }

            if (!config.setExposingOfChannelDataAsProperties(GetParam()))
                return nullptr;

            return m_logicEngine.createAnimationNode(config, name);
        }

        template <typename T>
        void advanceAnimationAndExpectValues(AnimationNode& animNode, float progress, const T& expectedValue)
        {
            EXPECT_TRUE(animNode.getInputs()->getChild("progress")->set(progress));
            EXPECT_TRUE(m_logicEngine.update());

            if constexpr (std::is_same_v<T, std::vector<float>>)
            {
                const auto outputArrayProp = animNode.getOutputs()->getChild("channel");
                ASSERT_EQ(outputArrayProp->getChildCount(), expectedValue.size());
                for (size_t i = 0u; i < outputArrayProp->getChildCount(); ++i)
                    EXPECT_FLOAT_EQ(expectedValue[i], *outputArrayProp->getChild(i)->get<float>());
            }
            else
            {
                const auto val = *animNode.getOutputs()->getChild("channel")->get<T>();
                if constexpr (std::is_same_v<T, vec2f>)
                {
                    EXPECT_FLOAT_EQ(expectedValue[0], val[0]);
                    EXPECT_FLOAT_EQ(expectedValue[1], val[1]);
                }
                else if constexpr (std::is_same_v<T, vec2i>)
                {
                    EXPECT_EQ(expectedValue[0], val[0]);
                    EXPECT_EQ(expectedValue[1], val[1]);
                }
                else if constexpr (std::is_same_v<T, vec4f>)
                {
                    EXPECT_FLOAT_EQ(expectedValue[0], val[0]);
                    EXPECT_FLOAT_EQ(expectedValue[1], val[1]);
                    EXPECT_FLOAT_EQ(expectedValue[2], val[2]);
                    EXPECT_FLOAT_EQ(expectedValue[3], val[3]);
                }
                else if constexpr (std::is_arithmetic_v<T>)
                {
                    EXPECT_EQ(expectedValue, val);
                }
                else
                {
                    ASSERT_TRUE(false) << "test missing for type";
                }
            }
        }

        void advanceAnimationAndExpectValues_twoChannels(AnimationNode& animNode, float progress, const vec2f& expectedValue1, const vec2f& expectedValue2)
        {
            animNode.getInputs()->getChild("progress")->set(progress);
            m_logicEngine.update();
            const auto val1 = *animNode.getOutputs()->getChild("channel1")->get<vec2f>();
            EXPECT_FLOAT_EQ(expectedValue1[0], val1[0]);
            EXPECT_FLOAT_EQ(expectedValue1[1], val1[1]);
            const auto val2 = *animNode.getOutputs()->getChild("channel2")->get<vec2f>();
            EXPECT_FLOAT_EQ(expectedValue2[0], val2[0]);
            EXPECT_FLOAT_EQ(expectedValue2[1], val2[1]);
        }

        LogicEngine m_logicEngine{ EFeatureLevel_Latest };
        DataArray* m_dataFloat = nullptr;
        DataArray* m_dataVec2 = nullptr;
        DataArray* m_dataVec4 = nullptr;
        DataArray* m_dataVecVec = nullptr;
        SaveFileConfig m_saveFileConfigNoValidation;
    };

    INSTANTIATE_TEST_SUITE_P(
        AnAnimationNode_TestInstances,
        AnAnimationNode,
        ::testing::Values(
            false, // without animation data exposed as properties
            true)  // with animation data exposed as properties
    );

    TEST_P(AnAnimationNode, IsCreated)
    {
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        const AnimationChannels channels{ channel, channel };
        const auto animNode = createAnimationNode(channels, "animNode");
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        ASSERT_NE(nullptr, animNode);
        EXPECT_EQ(animNode, m_logicEngine.findByName<AnimationNode>("animNode"));

        EXPECT_EQ("animNode", animNode->getName());
        EXPECT_EQ(channels, animNode->getChannels());
    }

    TEST_P(AnAnimationNode, IsDestroyed)
    {
        const auto animNode = createAnimationNode({ { "channel", m_dataFloat, m_dataVec2 } }, "animNode");
        EXPECT_TRUE(m_logicEngine.destroy(*animNode));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        EXPECT_EQ(nullptr, m_logicEngine.findByName<AnimationNode>("animNode"));
    }

    TEST_P(AnAnimationNode, FailsToBeDestroyedIfFromOtherLogicInstance)
    {
        auto animNode = createAnimationNode({ { "channel", m_dataFloat, m_dataVec2 } }, "animNode");

        LogicEngine otherEngine;
        EXPECT_FALSE(otherEngine.destroy(*animNode));
        ASSERT_FALSE(otherEngine.getErrors().empty());
        EXPECT_EQ("Can't find AnimationNode in logic engine!", otherEngine.getErrors().front().message);
    }

    TEST_P(AnAnimationNode, ChangesName)
    {
        const auto animNode = createAnimationNode({ { "channel", m_dataFloat, m_dataVec2 } }, "animNode");

        animNode->setName("an");
        EXPECT_EQ("an", animNode->getName());
        EXPECT_EQ(animNode, m_logicEngine.findByName<AnimationNode>("an"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());
    }

    TEST_P(AnAnimationNode, CanContainVariousAnimationChannels)
    {
        const auto timeStamps1 = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f });
        const auto timeStamps2 = m_logicEngine.createDataArray(std::vector<float>{ 3.f, 4.f, 5.f });
        const auto data1 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 11.f, 22.f }, { 33.f, 44.f } });
        const auto data2 = m_logicEngine.createDataArray(std::vector<vec2i>{ { 11, 22 }, { 44, 55 }, { 66, 77 } });

        const AnimationChannel channel1{ "channel1", timeStamps1, data1, EInterpolationType::Step };
        const AnimationChannel channel2{ "channel2", timeStamps1, data1, EInterpolationType::Linear };
        const AnimationChannel channel3{ "channel3", timeStamps2, data2, EInterpolationType::Linear };
        const AnimationChannel channel4{ "channel4", timeStamps1, data1, EInterpolationType::Cubic, data1, data1 };
        const AnimationChannels channels{ channel1, channel2, channel3, channel4 };

        const auto animNode = createAnimationNode(channels, "animNode");

        EXPECT_TRUE(m_logicEngine.getErrors().empty());
        ASSERT_NE(nullptr, animNode);
        EXPECT_EQ(animNode, m_logicEngine.findByName<AnimationNode>("animNode"));

        EXPECT_EQ("animNode", animNode->getName());
        EXPECT_FLOAT_EQ(5.f, *animNode->getOutputs()->getChild("duration")->get<float>());
        EXPECT_EQ(channels, animNode->getChannels());
    }

    TEST_P(AnAnimationNode, HasFixedProperties)
    {
        const AnimationChannel channel{ "channel", m_dataFloat, m_dataVec2 };
        const AnimationChannels channels{ channel, channel };
        const auto animNode = createAnimationNode(channels, "animNode");
        ASSERT_NE(nullptr, animNode);

        const auto rootIn = animNode->getInputs();
        ASSERT_TRUE(rootIn);
        EXPECT_EQ("", rootIn->getName());
        ASSERT_EQ(rootIn->getChildCount(), GetParam() ? 2u : 1u); // with/out animation data properties
        ASSERT_TRUE(rootIn->getChild("progress"));
        EXPECT_EQ(EPropertyType::Float, rootIn->getChild("progress")->getType());

        const auto rootOut = animNode->getOutputs();
        ASSERT_TRUE(rootOut);
        EXPECT_EQ("", rootOut->getName());
        ASSERT_EQ(3u, rootOut->getChildCount());
        ASSERT_TRUE(rootOut && rootOut->getChild("duration"));
        EXPECT_EQ(EPropertyType::Float, rootOut->getChild("duration")->getType());
    }

    TEST_P(AnAnimationNode, HasPropertiesMatchingChannels)
    {
        const AnimationChannel channel1{ "channel1", m_dataFloat, m_dataFloat };
        const AnimationChannel channel2{ "channel2", m_dataFloat, m_dataVec4, EInterpolationType::Linear_Quaternions };
        const auto animNode = createAnimationNode({ channel1, channel2 }, "animNode");

        const auto rootOut = animNode->getOutputs();
        ASSERT_EQ(3u, rootOut->getChildCount());
        EXPECT_EQ("channel1", rootOut->getChild(1u)->getName());
        EXPECT_EQ("channel2", rootOut->getChild(2u)->getName());
        EXPECT_EQ(EPropertyType::Float, rootOut->getChild(1u)->getType());
        EXPECT_EQ(EPropertyType::Vec4f, rootOut->getChild(2u)->getType());
    }

    TEST_P(AnAnimationNode, HasPropertiesMatchingChannelsWithFloatArrays)
    {
        if (GetParam()) // cannot use float arrays exposed as input properties
            GTEST_SKIP();

        const AnimationChannel channel1{ "channel1", m_dataFloat, m_dataFloat };
        const AnimationChannel channel2{ "channel2", m_dataFloat, m_dataVecVec };
        const auto animNode = createAnimationNode({ channel1, channel2 }, "animNode");

        const auto rootOut = animNode->getOutputs();
        ASSERT_EQ(3u, rootOut->getChildCount());
        EXPECT_EQ("channel1", rootOut->getChild(1u)->getName());
        EXPECT_EQ("channel2", rootOut->getChild(2u)->getName());
        EXPECT_EQ(EPropertyType::Float, rootOut->getChild(1u)->getType());
        EXPECT_EQ(EPropertyType::Array, rootOut->getChild(2u)->getType());

        const auto arrayOutProp = rootOut->getChild(2u);
        ASSERT_EQ(5u, arrayOutProp->getChildCount());
        for (size_t i = 0u; i < 5u; ++i)
            EXPECT_EQ(EPropertyType::Float, arrayOutProp->getChild(i)->getType());
    }

    TEST_P(AnAnimationNode, DeterminesDurationFromHighestTimestamp)
    {
        const auto timeStamps1 = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        const auto timeStamps2 = m_logicEngine.createDataArray(std::vector<float>{ 4.f, 5.f, 6.f });

        const auto animNode1 = createAnimationNode({ { "channel", timeStamps1, m_dataVec2 } }, "animNode1");
        EXPECT_FLOAT_EQ(3.f, *animNode1->getOutputs()->getChild("duration")->get<float>());
        const auto animNode2 = createAnimationNode({ { "channel1", timeStamps1, m_dataVec2 }, { "channel2", timeStamps2, m_dataVec2 } }, "animNode2");
        EXPECT_FLOAT_EQ(6.f, *animNode2->getOutputs()->getChild("duration")->get<float>());
    }

    TEST_P(AnAnimationNode, FailsToBeCreatedWithNoChannels)
    {
        EXPECT_EQ(nullptr, createAnimationNode({}, "animNode"));
        EXPECT_EQ("Failed to create AnimationNode 'animNode': must provide at least one channel.", m_logicEngine.getErrors().front().message);
    }

    TEST_P(AnAnimationNode, FailsToBeCreatedIfDataArrayFromOtherLogicInstance)
    {
        LogicEngine otherInstance;
        auto otherInstanceData = otherInstance.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });

        EXPECT_EQ(nullptr, createAnimationNode({ { "channel", otherInstanceData, m_dataFloat } }, "animNode"));
        EXPECT_EQ("Failed to create AnimationNode 'animNode': timestamps or keyframes were not found in this logic instance.", m_logicEngine.getErrors().front().message);
        EXPECT_EQ(nullptr, createAnimationNode({ { "channel", m_dataFloat, otherInstanceData } }, "animNode"));
        EXPECT_EQ("Failed to create AnimationNode 'animNode': timestamps or keyframes were not found in this logic instance.", m_logicEngine.getErrors().front().message);
        EXPECT_EQ(nullptr, createAnimationNode({ { "channel", m_dataFloat, m_dataFloat, EInterpolationType::Cubic, otherInstanceData, m_dataFloat } }, "animNode"));
        EXPECT_EQ("Failed to create AnimationNode 'animNode': tangents were not found in this logic instance.", m_logicEngine.getErrors().front().message);
        EXPECT_EQ(nullptr, createAnimationNode({ { "channel", m_dataFloat, m_dataFloat, EInterpolationType::Cubic, m_dataFloat, otherInstanceData } }, "animNode"));
        EXPECT_EQ("Failed to create AnimationNode 'animNode': tangents were not found in this logic instance.", m_logicEngine.getErrors().front().message);
    }

    TEST_P(AnAnimationNode, CanBeSerializedAndDeserialized)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine otherEngine{ EFeatureLevel_Latest };

            const auto timeStamps1 = otherEngine.createDataArray(std::vector<float>{ 1.f, 2.f }, "ts1");
            const auto timeStamps2 = otherEngine.createDataArray(std::vector<float>{ 3.f, 4.f, 5.f }, "ts2");
            const auto data1 = otherEngine.createDataArray(std::vector<vec2i>{ { 11, 22 }, { 33, 44 } }, "data1");
            const auto data2 = otherEngine.createDataArray(std::vector<vec2i>{ { 11, 22 }, { 44, 55 }, { 66, 77 } }, "data2");
            const auto data3 = otherEngine.createDataArray(*m_dataVecVec->getData<std::vector<float>>(), "data3");

            const AnimationChannel channel1{ "channel1", timeStamps1, data1, EInterpolationType::Step };
            const AnimationChannel channel2{ "channel2", timeStamps1, data1, EInterpolationType::Linear };
            const AnimationChannel channel3{ "channel3", timeStamps2, data2, EInterpolationType::Linear };
            const AnimationChannel channel4{ "channel4", timeStamps1, data1, EInterpolationType::Cubic, data1, data1 };
            const AnimationChannel channel5{ "channel5", timeStamps2, data3, EInterpolationType::Cubic, data3, data3 };

            AnimationNodeConfig config1;
            EXPECT_TRUE(config1.addChannel(channel1));
            EXPECT_TRUE(config1.addChannel(channel2));
            EXPECT_TRUE(config1.addChannel(channel3));
            EXPECT_TRUE(config1.addChannel(channel4));
            if (!GetParam())
            {
                EXPECT_TRUE(config1.addChannel(channel5));
            }
            EXPECT_TRUE(config1.setExposingOfChannelDataAsProperties(GetParam()));

            AnimationNodeConfig config2;
            EXPECT_TRUE(config2.addChannel(channel4));
            EXPECT_TRUE(config2.addChannel(channel3));
            EXPECT_TRUE(config2.addChannel(channel2));
            EXPECT_TRUE(config2.addChannel(channel1));
            if (!GetParam())
            {
                EXPECT_TRUE(config2.addChannel(channel5));
            }
            EXPECT_TRUE(config2.setExposingOfChannelDataAsProperties(GetParam()));

            otherEngine.createAnimationNode(config1, "animNode1");
            otherEngine.createAnimationNode(config2, "animNode2");

            ASSERT_TRUE(otherEngine.saveToFile("logic_animNodes.bin", m_saveFileConfigNoValidation));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_animNodes.bin"));
        EXPECT_TRUE(m_logicEngine.getErrors().empty());

        EXPECT_EQ(2u, m_logicEngine.getCollection<AnimationNode>().size());
        const auto animNode1 = m_logicEngine.findByName<AnimationNode>("animNode1");
        const auto animNode2 = m_logicEngine.findByName<AnimationNode>("animNode2");
        ASSERT_TRUE(animNode1 && animNode2);

        EXPECT_EQ("animNode1", animNode1->getName());
        EXPECT_EQ("animNode2", animNode2->getName());
        EXPECT_FLOAT_EQ(5.f, *animNode1->getOutputs()->getChild("duration")->get<float>());
        EXPECT_FLOAT_EQ(5.f, *animNode2->getOutputs()->getChild("duration")->get<float>());

        // ptrs are different after loading, find data arrays to verify the references in loaded anim nodes match
        const auto ts1 = m_logicEngine.findByName<DataArray>("ts1");
        const auto ts2 = m_logicEngine.findByName<DataArray>("ts2");
        const auto data1 = m_logicEngine.findByName<DataArray>("data1");
        const auto data2 = m_logicEngine.findByName<DataArray>("data2");
        const auto data3 = m_logicEngine.findByName<DataArray>("data3");
        const AnimationChannel channel1{ "channel1", ts1, data1, EInterpolationType::Step };
        const AnimationChannel channel2{ "channel2", ts1, data1, EInterpolationType::Linear };
        const AnimationChannel channel3{ "channel3", ts2, data2, EInterpolationType::Linear };
        const AnimationChannel channel4{ "channel4", ts1, data1, EInterpolationType::Cubic, data1, data1 };
        const AnimationChannel channel5{ "channel5", ts2, data3, EInterpolationType::Cubic, data3, data3 };
        AnimationChannels expectedChannels1{ channel1, channel2, channel3, channel4 };
        AnimationChannels expectedChannels2{ channel4, channel3, channel2, channel1 };
        if (!GetParam())
        {
            expectedChannels1.push_back(channel5);
            expectedChannels2.push_back(channel5);
        }

        EXPECT_EQ(expectedChannels1, animNode1->getChannels());
        EXPECT_EQ(expectedChannels2, animNode2->getChannels());

        // verify properties after loading
        // check properties same for both anim nodes
        for (const auto animNode : { animNode1, animNode2 })
        {
            const auto rootIn = animNode->getInputs();
            EXPECT_EQ("", rootIn->getName());
            ASSERT_EQ(rootIn->getChildCount(), GetParam() ? 2u : 1u); // with/out animation data properties
            EXPECT_EQ("progress", rootIn->getChild(0u)->getName());
            EXPECT_EQ(EPropertyType::Float, rootIn->getChild(0u)->getType());

            const auto rootOut = animNode->getOutputs();
            EXPECT_EQ("", rootOut->getName());
            ASSERT_EQ((GetParam() ? 5u : 6u), rootOut->getChildCount());
            ASSERT_EQ("duration", rootOut->getChild(0u)->getName());
            ASSERT_EQ(EPropertyType::Float, rootOut->getChild(0u)->getType());
            EXPECT_FLOAT_EQ(5.f, *animNode->getOutputs()->getChild("duration")->get<float>());

            EXPECT_EQ(EPropertyType::Vec2i, rootOut->getChild(1u)->getType());
            EXPECT_EQ(EPropertyType::Vec2i, rootOut->getChild(2u)->getType());
            EXPECT_EQ(EPropertyType::Vec2i, rootOut->getChild(3u)->getType());
            EXPECT_EQ(EPropertyType::Vec2i, rootOut->getChild(4u)->getType());

            if (!GetParam())
            {
                const auto arrayOutProp = rootOut->getChild(5u);
                EXPECT_EQ(EPropertyType::Array, arrayOutProp->getType());
                ASSERT_EQ(5u, arrayOutProp->getChildCount());
                for (size_t i = 0u; i < 5u; ++i)
                    EXPECT_EQ(EPropertyType::Float, arrayOutProp->getChild(i)->getType());
            }
        }
        // check output names separately
        const auto rootOut1 = animNode1->getOutputs();
        EXPECT_EQ("channel1", rootOut1->getChild(1u)->getName());
        EXPECT_EQ("channel2", rootOut1->getChild(2u)->getName());
        EXPECT_EQ("channel3", rootOut1->getChild(3u)->getName());
        EXPECT_EQ("channel4", rootOut1->getChild(4u)->getName());
        if (!GetParam())
        {
            EXPECT_EQ("channel5", rootOut1->getChild(5u)->getName());
        }

        const auto rootOut2 = animNode2->getOutputs();
        EXPECT_EQ("channel4", rootOut2->getChild(1u)->getName());
        EXPECT_EQ("channel3", rootOut2->getChild(2u)->getName());
        EXPECT_EQ("channel2", rootOut2->getChild(3u)->getName());
        EXPECT_EQ("channel1", rootOut2->getChild(4u)->getName());
        if (!GetParam())
        {
            EXPECT_EQ("channel5", rootOut2->getChild(5u)->getName());
        }
    }

    TEST_P(AnAnimationNode, WillSerializeAnimationIncludingProgress)
    {
        WithTempDirectory tempDir;

        {
            LogicEngine otherEngine{ EFeatureLevel_Latest };

            const auto timeStamps = otherEngine.createDataArray(std::vector<float>{ 1.f, 2.f }, "ts");
            const auto data = otherEngine.createDataArray(std::vector<int32_t>{ 10, 20 }, "data");
            const AnimationChannel channel{ "channel", timeStamps, data, EInterpolationType::Linear };
            AnimationNodeConfig config;
            config.addChannel(channel);
            config.setExposingOfChannelDataAsProperties(GetParam());
            const auto animNode = otherEngine.createAnimationNode(config, "animNode");

            EXPECT_TRUE(animNode->getInputs()->getChild("progress")->set(0.75f));
            EXPECT_TRUE(otherEngine.update());
            EXPECT_EQ(15, *animNode->getOutputs()->getChild("channel")->get<int32_t>());

            ASSERT_TRUE(otherEngine.saveToFile("logic_animNodes.bin", m_saveFileConfigNoValidation));
        }

        ASSERT_TRUE(m_logicEngine.loadFromFile("logic_animNodes.bin"));
        const auto animNode = m_logicEngine.findByName<AnimationNode>("animNode");
        ASSERT_TRUE(animNode);

        // update node with no change to progress to check its state
        EXPECT_TRUE(m_logicEngine.update());

        // progress same as when saved
        EXPECT_EQ(15, *animNode->getOutputs()->getChild("channel")->get<int32_t>());

        // can play again
        advanceAnimationAndExpectValues<int32_t>(*animNode, 0.f,   10);
        advanceAnimationAndExpectValues<int32_t>(*animNode, 0.5f,  10);
        advanceAnimationAndExpectValues<int32_t>(*animNode, 0.75f, 15);
        advanceAnimationAndExpectValues<int32_t>(*animNode, 1.f,   20);
    }

    TEST_P(AnAnimationNode, CanHandleProgressOutOfNormalizedRange)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<float>{ 10.f, 20.f });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues<float>(*animNode, 0.0f, 10.f);
        advanceAnimationAndExpectValues<float>(*animNode, 0.5f, 15.f);
        advanceAnimationAndExpectValues<float>(*animNode, 1.0f, 20.f);
        advanceAnimationAndExpectValues<float>(*animNode, -999.f, 10.f);
        advanceAnimationAndExpectValues<float>(*animNode, 999.f, 20.f);
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_step_vec2f)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2f>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Step } });

        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.f, { 0.f, 10.f });
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.99f, { 0.f, 10.f }); // still no change
        advanceAnimationAndExpectValues<vec2f>(*animNode, 1.000001f, { 1.f, 20.f }); // step to next keyframe value at its timestamp
        advanceAnimationAndExpectValues<vec2f>(*animNode, 100.f, { 1.f, 20.f }); // no change pass end of animation
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_step_vec2i)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2i>{ { 0, 10 }, { 1, 20 } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Step } });

        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.f, { 0, 10 });
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.99f, { 0, 10 }); // still no change
        advanceAnimationAndExpectValues<vec2i>(*animNode, 1.000001f, { 1, 20 }); // step to next keyframe value at its timestamp
        advanceAnimationAndExpectValues<vec2i>(*animNode, 100.f, { 1, 20 }); // no change pass end of animation
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_step_vecvec)
    {
        if (GetParam())
            GTEST_SKIP();

        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Step } });

        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.f, { 0.f, 10.f });
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.99f, { 0.f, 10.f }); // still no change
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 1.000001f, { 1.f, 20.f }); // step to next keyframe value at its timestamp
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 100.f, { 1.f, 20.f }); // no change pass end of animation
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_linear_vec2f)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2f>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.f, { 0.f, 10.f });
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.1f, { 0.1f, 11.f }); // time 0.1
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.5f, { 0.5f, 15.f }); // time 0.5
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.9f, { 0.9f, 19.f }); // time 0.9
        advanceAnimationAndExpectValues<vec2f>(*animNode, 1.0f, { 1.f, 20.f }); // time 1.0
        advanceAnimationAndExpectValues<vec2f>(*animNode, 100.f, { 1.f, 20.f }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_linear_vec2i)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2i>{ { 0, 10 }, { 1, 20 } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.f, { 0, 10 });
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.1f, { 0, 11 }); // time 0.1
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.5f, { 1, 15 }); // time 0.5
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.9f, { 1, 19 }); // time 0.9
        advanceAnimationAndExpectValues<vec2i>(*animNode, 1.0f, { 1, 20 }); // time 1.0
        advanceAnimationAndExpectValues<vec2i>(*animNode, 100.f, { 1, 20 }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_linear_vecvec)
    {
        if (GetParam())
            GTEST_SKIP();

        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.f, { 0.f, 10.f });
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.1f, { 0.1f, 11.f }); // time 0.1
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.5f, { 0.5f, 15.f }); // time 0.5
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.9f, { 0.9f, 19.f }); // time 0.9
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 1.0f, { 1.f, 20.f }); // time 1.0
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 100.f, { 1.f, 20.f }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_linear_quaternions)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, m_dataVec4, EInterpolationType::Linear_Quaternions } });

        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.f, { 1, 0, 0, 0 });
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.25f, { .70710677f, .70710677f, 0, 0 }); // time 0.5
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.5f, { 0, 1, 0, 0 }); // time 1.0
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.75f, { 0, .70710677f, .70710677f, 0 }); // time 1.5
        advanceAnimationAndExpectValues<vec4f>(*animNode, 100.f, { 0, 0, 1, 0 }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_cubic_vec2f)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2f>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto tangentsZero = m_logicEngine.createDataArray(std::vector<vec2f>{ { 0.f, 0.f }, { 0.f, 0.f } });
        const auto tangentsIn = m_logicEngine.createDataArray(std::vector<vec2f>{ { 0.f, 0.f }, { -1.f, -2.f } });
        const auto tangentsOut = m_logicEngine.createDataArray(std::vector<vec2f>{ { 2.f, 5.f }, { 0.f, 0.f } });
        // animation with one channel using zero tangents and another channel with non-zero tangents
        const auto animNode = createAnimationNode({
            { "channel1", timeStamps, data, EInterpolationType::Cubic, tangentsZero, tangentsZero },
            { "channel2", timeStamps, data, EInterpolationType::Cubic, tangentsIn, tangentsOut },
            });

        advanceAnimationAndExpectValues_twoChannels(*animNode, 0.f, { 0.f, 10.f }, { 0.f, 10.f });
        advanceAnimationAndExpectValues_twoChannels(*animNode, 0.1f, { 0.028f, 10.28f }, { 0.199f, 10.703f }); // time 0.1
        advanceAnimationAndExpectValues_twoChannels(*animNode, 0.5f, { 0.5f, 15.f }, { 0.875f, 15.875f }); // time 0.5
        advanceAnimationAndExpectValues_twoChannels(*animNode, 0.9f, { 0.972f, 19.72f }, { 1.071f, 19.927f }); // time 0.9
        advanceAnimationAndExpectValues_twoChannels(*animNode, 1.0f, { 1.f, 20.f }, { 1.f, 20.f }); // time 1.0
        advanceAnimationAndExpectValues_twoChannels(*animNode, 100.f, { 1.f, 20.f }, { 1.f, 20.f }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_cubic_vecvec)
    {
        if (GetParam())
            GTEST_SKIP();

        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 0.f, 10.f }, { 1.f, 20.f } });
        const auto tangentsIn = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 0.f, 0.f }, { -1.f, -2.f } });
        const auto tangentsOut = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 2.f, 5.f }, { 0.f, 0.f } });
        const auto animNode = createAnimationNode({{ "channel", timeStamps, data, EInterpolationType::Cubic, tangentsIn, tangentsOut }});

        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.f, { 0.f, 10.f });
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.1f, { 0.199f, 10.703f }); // time 0.1
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.5f, { 0.875f, 15.875f }); // time 0.5
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 0.9f, { 1.071f, 19.927f }); // time 0.9
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 1.0f, { 1.f, 20.f }); // time 1.0
        advanceAnimationAndExpectValues<std::vector<float>>(*animNode, 100.f, { 1.f, 20.f }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_cubic_quaternions)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f });
        const auto tangentsZero = m_logicEngine.createDataArray(std::vector<vec4f>{ { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } });
        const auto animNode = createAnimationNode({{ "channel", timeStamps, m_dataVec4, EInterpolationType::Cubic_Quaternions, tangentsZero, tangentsZero }});

        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.f, { 1, 0, 0, 0 });
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.125f, { .98328203f, .18208927f, 0, 0 }); // time 0.25
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.25f, { .70710677f, .70710677f, 0, 0 }); // time 0.5
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.375f, { .18208927f, .98328203f, 0, 0 }); // time 0.75
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.5f, { 0, 1, 0, 0 }); // time 1.0
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.875f, { 0, .18208927f, .98328203f, 0 }); // time 1.75
        advanceAnimationAndExpectValues<vec4f>(*animNode, 100.f, { 0, 0, 1, 0 }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_cubic_quaternions_withTangents)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f });
        const auto tangentsIn = m_logicEngine.createDataArray(std::vector<vec4f>{ { 0.f, 0.f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.f }, { 1.f, 1.f, 0.f, 0.f } });
        const auto tangentsOut = m_logicEngine.createDataArray(std::vector<vec4f>{ { 1.f, 1.f, 0.f, 0.f}, {  1.f, 1.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, m_dataVec4, EInterpolationType::Cubic_Quaternions, tangentsIn, tangentsOut } });

        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.f, { 1, 0, 0, 0 });
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.125f, { 0.9749645f, 0.22236033f, 0, 0 }); // time 0.25
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.25f, { .70710677f, .70710677f, 0, 0 }); // time 0.5
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.375f, { 0.13598002f, 0.99071163f, 0, 0 }); // time 0.75
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.5f, { 0, 1, 0, 0 }); // time 1.0
        advanceAnimationAndExpectValues<vec4f>(*animNode, 0.875f, { -0.055011157f, 0.12835936f, 0.99020082f, 0 }); // time 1.75
        advanceAnimationAndExpectValues<vec4f>(*animNode, 100.f, { 0, 0, 1, 0 }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatesKeyframeValues_cubic_vec2i)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2i>{ { 0, 10 }, { 1, 20 } });
        const auto tangentsIn = m_logicEngine.createDataArray(std::vector<vec2i>{ { 0, 0 }, { -1, -2 } });
        const auto tangentsOut = m_logicEngine.createDataArray(std::vector<vec2i>{ { 2, 5 }, { 0, 0 } });
        const auto animNode = createAnimationNode({
            { "channel", timeStamps, data, EInterpolationType::Cubic, tangentsIn, tangentsOut },
            });

        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.f, { 0, 10 });
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.1f, { 0, 11 }); // time 0.1
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.5f, { 1, 16 }); // time 0.5
        advanceAnimationAndExpectValues<vec2i>(*animNode, 0.9f, { 1, 20 }); // time 0.9
        advanceAnimationAndExpectValues<vec2i>(*animNode, 1.0f, { 1, 20 }); // time 1.0
        advanceAnimationAndExpectValues<vec2i>(*animNode, 100.f, { 1, 20 }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, InterpolatedValueBeforeFirstTimestampIsFirstKeyframe)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f });
        const auto data = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 20.f }, { 2.f, 30.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.f, { 1.f, 20.f });
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.25f, { 1.f, 20.f }); // time 0.5
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.5f, { 1.f, 20.f }); // time 1.0
        advanceAnimationAndExpectValues<vec2f>(*animNode, 0.75f, { 1.5f, 25.f }); // time 1.5
        advanceAnimationAndExpectValues<vec2f>(*animNode, 100.f, { 2.f, 30.f }); // stays at last keyframe after animation end
    }

    TEST_P(AnAnimationNode, CanJumpAnywhereInAnimation)
    {
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
        const auto data = m_logicEngine.createDataArray(std::vector<float>{ { 10.f, 20.f } });
        const auto animNode = createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        advanceAnimationAndExpectValues(*animNode, 0.f, 10.f);
        advanceAnimationAndExpectValues(*animNode, 1.f, 20.f);
        advanceAnimationAndExpectValues(*animNode, -10.f, 10.f);
        advanceAnimationAndExpectValues(*animNode, 10.f, 20.f);
        advanceAnimationAndExpectValues(*animNode, 0.5f, 15.f);
        advanceAnimationAndExpectValues(*animNode, 0.25f, 12.5f);
        advanceAnimationAndExpectValues(*animNode, 0.75f, 17.5f);
    }

    TEST_P(AnAnimationNode, GivesStableResultsWithExtremelySmallTimestamps)
    {
        constexpr float Eps = std::numeric_limits<float>::epsilon();
        const auto timeStamps = m_logicEngine.createDataArray(std::vector<float>{ Eps * 100, Eps * 200 });
        const auto data = m_logicEngine.createDataArray(std::vector<float>{ { 1.f,  2.f } });
        auto& animNode = *createAnimationNode({ { "channel", timeStamps, data, EInterpolationType::Linear } });

        // initialize output value by updating with zero progress and expect first keyframe value
        advanceAnimationAndExpectValues(animNode, 0.f, 1.f);

        float lastValue = 0.f;
        for (int i = 0; i < 110; ++i)
        {
            const float progress = 0.01f * float(i);

            animNode.getInputs()->getChild("progress")->set(progress);
            m_logicEngine.update();
            const auto val = *animNode.getOutputs()->getChild("channel")->get<float>();

            // expect interpolated value to go through keyframes in stable manner
            EXPECT_TRUE(val >= lastValue);
            lastValue = val;
        }
        // expect last keyframe value
        EXPECT_FLOAT_EQ(2.f, lastValue);
    }

    TEST_P(AnAnimationNode, CanBeCreatedWithMoreThanMaximumArraySizeKeyframesIfNotExposedViaProperties)
    {
        std::vector<float> vecDataExceeding;
        vecDataExceeding.resize(MaxArrayPropertySize + 1u);
        std::iota(vecDataExceeding.begin(), vecDataExceeding.end(), 1.f);
        const auto dataExceeding = m_logicEngine.createDataArray(vecDataExceeding);

        const AnimationChannel channelExceeding{ "channel2", dataExceeding, dataExceeding };
        AnimationNodeConfig config;
        config.addChannel(channelExceeding);
        EXPECT_NE(nullptr, m_logicEngine.createAnimationNode(config, "animNode"));
    }

    class AnAnimationNode_SerializationLifecycle : public AnAnimationNode
    {
    protected:
        enum class ESerializationIssue
        {
            AllValid,
            NameMissing,
            IdMissing,
            ChannelsMissing,
            RootInMissing,
            RootOutMissing,
            ChannelNameMissing,
            ChannelTimestampsMissing,
            ChannelKeyframesMissing,
            ChannelTangentsInMissing,
            ChannelTangentsOutMissing,
            InvalidInterpolationType,
            PropertyInMissing,
            PropertyOutMissing,
            PropertyInWrongName,
            PropertyOutWrongName,
            PropertyChannelsDataInvalid
        };

        std::unique_ptr<AnimationNodeImpl> deserializeSerializedDataWithIssue(ESerializationIssue issue)
        {
            flatbuffers::FlatBufferBuilder flatBufferBuilder;
            SerializationMap serializationMap;
            DeserializationMap deserializationMap;

            {
                const auto data = m_logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });

                HierarchicalTypeData inputs = MakeStruct("", {});
                if (issue != ESerializationIssue::PropertyInMissing)
                {
                    if (issue == ESerializationIssue::PropertyInWrongName)
                    {
                        inputs.children.push_back(MakeType("wrongInput", EPropertyType::Float));
                    }
                    else
                    {
                        inputs.children.push_back(MakeType("progress", EPropertyType::Float));
                    }
                }
                if (issue == ESerializationIssue::PropertyChannelsDataInvalid)
                    inputs.children.push_back(MakeType("invalidChannelsData", EPropertyType::Array));

                auto inputsImpl = std::make_unique<PropertyImpl>(std::move(inputs), EPropertySemantics::AnimationInput);

                HierarchicalTypeData outputs = MakeStruct("", {});
                if (issue == ESerializationIssue::PropertyOutWrongName)
                {
                    outputs.children.push_back(MakeType("wrongOutput", EPropertyType::Float));
                }
                else
                {
                    outputs.children.push_back(MakeType("duration", EPropertyType::Float));
                }
                if (issue != ESerializationIssue::PropertyOutMissing)
                    outputs.children.push_back(MakeType("channel", EPropertyType::Float));
                auto outputsImpl = std::make_unique<PropertyImpl>(std::move(outputs), EPropertySemantics::AnimationOutput);

                const auto dataFb = DataArrayImpl::Serialize(data->m_impl, flatBufferBuilder, serializationMap, m_logicEngine.getFeatureLevel());
                flatBufferBuilder.Finish(dataFb);
                const auto dataFbSerialized = flatbuffers::GetRoot<rlogic_serialization::DataArray>(flatBufferBuilder.GetBufferPointer());
                deserializationMap.storeDataArray(*dataFbSerialized, *data);

                std::vector<flatbuffers::Offset<rlogic_serialization::Channel>> channelsFB;
                channelsFB.push_back(rlogic_serialization::CreateChannel(
                    flatBufferBuilder,
                    issue == ESerializationIssue::ChannelNameMissing ? 0 : flatBufferBuilder.CreateString("channel"),
                    issue == ESerializationIssue::ChannelTimestampsMissing ? 0 : dataFb,
                    issue == ESerializationIssue::ChannelKeyframesMissing ? 0 : dataFb,
                    issue == ESerializationIssue::InvalidInterpolationType ? static_cast<rlogic_serialization::EInterpolationType>(10) : rlogic_serialization::EInterpolationType::Cubic,
                    issue == ESerializationIssue::ChannelTangentsInMissing ? 0 : dataFb,
                    issue == ESerializationIssue::ChannelTangentsOutMissing ? 0 : dataFb
                ));

                const auto animNodeFB = rlogic_serialization::CreateAnimationNode(
                    flatBufferBuilder,
                    rlogic_serialization::CreateLogicObject(flatBufferBuilder,
                        issue == ESerializationIssue::NameMissing ? 0 : flatBufferBuilder.CreateString("animNode"),
                        issue == ESerializationIssue::IdMissing ? 0 : 1u),
                    issue == ESerializationIssue::ChannelsMissing ? 0 : flatBufferBuilder.CreateVector(channelsFB),
                    issue == ESerializationIssue::PropertyChannelsDataInvalid,
                    issue == ESerializationIssue::RootInMissing ? 0 : PropertyImpl::Serialize(*inputsImpl, flatBufferBuilder, serializationMap),
                    issue == ESerializationIssue::RootOutMissing ? 0 : PropertyImpl::Serialize(*outputsImpl, flatBufferBuilder, serializationMap)
                );

                flatBufferBuilder.Finish(animNodeFB);
            }

            const auto& serialized = *flatbuffers::GetRoot<rlogic_serialization::AnimationNode>(flatBufferBuilder.GetBufferPointer());
            return AnimationNodeImpl::Deserialize(serialized, m_errorReporting, deserializationMap);
        }

        ErrorReporting m_errorReporting;
    };

    INSTANTIATE_TEST_SUITE_P(
        AnAnimationNode_SerializationLifecycle_TestInstances,
        AnAnimationNode_SerializationLifecycle,
        ::testing::Values(false, true));

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfEssentialDataMissing)
    {
        EXPECT_TRUE(deserializeSerializedDataWithIssue(AnAnimationNode_SerializationLifecycle::ESerializationIssue::AllValid));
        EXPECT_TRUE(m_errorReporting.getErrors().empty());

        for (const auto issue : { ESerializationIssue::NameMissing, ESerializationIssue::IdMissing, ESerializationIssue::ChannelsMissing, ESerializationIssue::RootInMissing, ESerializationIssue::RootOutMissing })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of AnimationNode from serialized data: missing name, id, channels or in/out property data!", m_errorReporting.getErrors().back().message);
            m_errorReporting.clear();
        }
    }

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfChannelDataMissing)
    {
        for (const auto issue : { ESerializationIssue::ChannelTimestampsMissing, ESerializationIssue::ChannelKeyframesMissing })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of AnimationNode 'animNode' channel data: missing name, timestamps or keyframes!", m_errorReporting.getErrors().front().message);
            m_errorReporting.clear();
        }
    }

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfTangentsMissing)
    {
        for (const auto issue : { ESerializationIssue::ChannelTangentsInMissing, ESerializationIssue::ChannelTangentsOutMissing })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of AnimationNode 'animNode' channel 'channel' data: missing tangents!", m_errorReporting.getErrors().front().message);
            m_errorReporting.clear();
        }
    }

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfInvalidInterpolationType)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ESerializationIssue::InvalidInterpolationType));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of AnimationNode 'animNode' channel 'channel' data: missing or invalid interpolation type!", m_errorReporting.getErrors().front().message);
        m_errorReporting.clear();
    }

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfInvalidOrMissingProperties)
    {
        for (const auto issue : { ESerializationIssue::PropertyInMissing, ESerializationIssue::PropertyOutMissing, ESerializationIssue::PropertyInWrongName, ESerializationIssue::PropertyOutWrongName })
        {
            EXPECT_FALSE(deserializeSerializedDataWithIssue(issue));
            ASSERT_FALSE(m_errorReporting.getErrors().empty());
            EXPECT_EQ("Fatal error during loading of AnimationNode 'animNode': missing or invalid properties!", m_errorReporting.getErrors().front().message);
            m_errorReporting.clear();
        }
    }

    TEST_P(AnAnimationNode_SerializationLifecycle, FailsDeserializationIfInvalidChannelsData)
    {
        EXPECT_FALSE(deserializeSerializedDataWithIssue(ESerializationIssue::PropertyChannelsDataInvalid));
        ASSERT_FALSE(m_errorReporting.getErrors().empty());
        EXPECT_EQ("Fatal error during loading of AnimationNode 'animNode': missing or invalid channels data property!", m_errorReporting.getErrors().front().message);
    }
}
