//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/DataArray.h"
#include <numeric>

namespace rlogic::internal
{
    class AnAnimationNodeConfig : public ::testing::Test
    {
    public:
        void SetUp() override
        {
            m_dataFloat = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
            m_dataVec2 = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
            m_dataVecVec = m_logicEngine.createDataArray(std::vector<std::vector<float>>{ { 1.f, 2.f }, { 3.f, 4.f }, { 5.f, 6.f } });
        }

        static bool createConfig(const std::initializer_list<AnimationChannel> channels)
        {
            AnimationNodeConfig config;
            for (const auto& channel : channels)
            {
                if (!config.addChannel(channel))
                    return false;
            }

            return true;
        }

    protected:
        LogicEngine m_logicEngine{ EFeatureLevel_Latest };
        DataArray* m_dataFloat = nullptr;
        DataArray* m_dataVec2 = nullptr;
        DataArray* m_dataVecVec = nullptr;
    };

    TEST_F(AnAnimationNodeConfig, CanBeCreatedWithValidChannels)
    {
        const AnimationChannel validChannel1{ "ok1", m_dataFloat, m_dataVec2 };
        const AnimationChannel validChannel2{ "ok2", m_dataFloat, m_dataVecVec };
        EXPECT_TRUE(createConfig({ validChannel1, validChannel2 }));
    }

    TEST_F(AnAnimationNodeConfig, CanBeCopiedAndMoved)
    {
        const AnimationChannel validChannel1{ "ok1", m_dataFloat, m_dataVec2 };
        const AnimationChannel validChannel2{ "ok2", m_dataFloat, m_dataVecVec };
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel(validChannel1));
        EXPECT_TRUE(config.addChannel(validChannel2));

        // assign
        AnimationNodeConfig config2;
        config2 = config;
        EXPECT_EQ(config.getChannels(), config2.getChannels());

        // copy ctor
        AnimationNodeConfig config3{ config };
        EXPECT_EQ(config.getChannels(), config3.getChannels());

        // move assign
        AnimationNodeConfig config4;
        config4 = std::move(config2);
        EXPECT_EQ(config.getChannels(), config4.getChannels());

        // move ctor
        AnimationNodeConfig config5{ std::move(config3) };
        EXPECT_EQ(config.getChannels(), config5.getChannels());
    }

    TEST_F(AnAnimationNodeConfig, CanGetAddedChannels)
    {
        AnimationNodeConfig config;
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        EXPECT_TRUE(config.addChannel(validChannel));
        AnimationChannels expected{ validChannel };
        EXPECT_EQ(expected, config.getChannels());

        EXPECT_TRUE(config.addChannel(validChannel));
        expected = { validChannel, validChannel };
        EXPECT_EQ(expected, config.getChannels());
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfMissingTimestampsOrKeyframes)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        EXPECT_FALSE(createConfig({ validChannel, { "channel", nullptr, m_dataVec2 } }));
        EXPECT_FALSE(createConfig({ { "channel", m_dataFloat, nullptr }, validChannel }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfTimestampsOrKeyframesTypeInvalid)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };
        const auto dataVec2OtherSize = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f } }); // single element only

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataVec2, m_dataVec2 } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, dataVec2OtherSize } }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfTimestampsNotStrictlyAscending)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        const auto timeStampsDescending = m_logicEngine.createDataArray(std::vector<float>{ { 1.f, 3.f, 2.f } });
        EXPECT_FALSE(createConfig({ validChannel, { "channel", timeStampsDescending, m_dataVec2 } }));

        const auto timeStampsNotStrictAscend = m_logicEngine.createDataArray(std::vector<float>{ { 1.f, 2.f, 2.f } });
        EXPECT_FALSE(createConfig({ validChannel, { "channel", timeStampsNotStrictAscend, m_dataVec2 } }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfTangentsProvidedForNonCubicInterpolation)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Linear, m_dataVec2, nullptr } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Linear, nullptr, m_dataVec2 } }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfQuaternionInterpolationWithNonVec4fKeyframes)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Linear_Quaternions } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic_Quaternions, m_dataVec2, m_dataVec2 } }));
        EXPECT_FALSE(createConfig({ { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic_Quaternions, m_dataVec2, m_dataVec2 }, validChannel }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfInputRequirementsNotMet_specificToCubicInerpolation)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataVec2, m_dataVec2 };
        EXPECT_TRUE(createConfig({ validChannel }));

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataVec2, nullptr } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, nullptr, m_dataVec2 } }));

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataVec2, m_dataFloat } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataFloat, m_dataVec2 } }));

        const auto dataVec2OtherSize = m_logicEngine.createDataArray(std::vector<vec2f>{ { 1.f, 2.f } }); // single element only
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataVec2, dataVec2OtherSize } }));
        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, dataVec2OtherSize, m_dataVec2 } }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfElementArraysTooBig)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2 };

        std::vector<std::vector<float>> tooBigArrays;
        tooBigArrays.resize(3u);
        for (auto& elementArray : tooBigArrays)
            elementArray.resize(MaxArrayPropertySize + 1u, 0.f);
        const auto invalidData = m_logicEngine.createDataArray(tooBigArrays, "invalid");

        EXPECT_FALSE(createConfig({ validChannel, { "channel", m_dataFloat, invalidData } }));
        EXPECT_FALSE(createConfig({ { "channel", m_dataFloat, invalidData }, validChannel }));
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfElementArraysSizeMismatchBetweenKeyframesAndTangents)
    {
        auto dataWithLargerElementArrays = *m_dataVecVec->getData<std::vector<float>>();
        for (auto& elementArray : dataWithLargerElementArrays)
            elementArray.push_back(0.f);
        const auto largerData = m_logicEngine.createDataArray(dataWithLargerElementArrays, "invalid");

        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVecVec, EInterpolationType::Cubic, m_dataVecVec, m_dataVecVec };
        const AnimationChannel invalidChannel1{ "invalid", m_dataFloat, m_dataVecVec, EInterpolationType::Cubic, m_dataVecVec, largerData };
        const AnimationChannel invalidChannel2{ "invalid", m_dataFloat, largerData, EInterpolationType::Cubic, m_dataVecVec, m_dataVecVec };

        EXPECT_FALSE(createConfig({ validChannel, invalidChannel1 }));
        EXPECT_FALSE(createConfig({ invalidChannel2, validChannel }));
    }

    TEST_F(AnAnimationNodeConfig, CanEnableAndDisableDataExposingAsPropeties)
    {
        const AnimationChannel validChannel{ "ok", m_dataFloat, m_dataVec2, EInterpolationType::Cubic, m_dataVec2, m_dataVec2 };

        AnimationNodeConfig config;
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(false));
        EXPECT_FALSE(config.getExposingOfChannelDataAsProperties());

        EXPECT_TRUE(config.addChannel(validChannel));
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(false));
        EXPECT_FALSE(config.getExposingOfChannelDataAsProperties());

        EXPECT_TRUE(config.addChannel(validChannel));
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(false));
        EXPECT_FALSE(config.getExposingOfChannelDataAsProperties());
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfDataExposeEnabledAndNumberOfKeyframesExceedsMaximum)
    {
        const auto dataOk = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        std::vector<float> vecDataExceeding;
        vecDataExceeding.resize(MaxArrayPropertySize + 1u);
        std::iota(vecDataExceeding.begin(), vecDataExceeding.end(), 1.f);
        const auto dataExceeding = m_logicEngine.createDataArray(vecDataExceeding);

        const AnimationChannel channelOk{ "channel1", dataOk, dataOk };
        const AnimationChannel channelExceeding{ "channel2", dataExceeding, dataExceeding };

        AnimationNodeConfig config;
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());

        EXPECT_TRUE(config.addChannel(channelOk));
        EXPECT_FALSE(config.addChannel(channelExceeding));

        // stays enabled
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());
    }

    TEST_F(AnAnimationNodeConfig, FailsToEnableDataExposeIfNumberOfKeyframesExceedsMaximum)
    {
        const auto dataOk = m_logicEngine.createDataArray(std::vector<float>{ 1.f, 2.f, 3.f });
        std::vector<float> vecDataExceeding;
        vecDataExceeding.resize(MaxArrayPropertySize + 1u);
        std::iota(vecDataExceeding.begin(), vecDataExceeding.end(), 1.f);
        const auto dataExceeding = m_logicEngine.createDataArray(vecDataExceeding);

        const AnimationChannel channelOk{ "channel1", dataOk, dataOk };
        const AnimationChannel channelExceeding{ "channel2", dataExceeding, dataExceeding };

        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel(channelOk));
        EXPECT_TRUE(config.addChannel(channelExceeding));

        EXPECT_FALSE(config.setExposingOfChannelDataAsProperties(true));
        // stays disabled
        EXPECT_FALSE(config.getExposingOfChannelDataAsProperties());
    }

    TEST_F(AnAnimationNodeConfig, FailsToAddChannelIfDataExposeEnabledAndAddingElementsOfArrayType)
    {
        AnimationNodeConfig config;
        EXPECT_TRUE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());

        EXPECT_FALSE(config.addChannel({ "channel", m_dataFloat, m_dataVecVec }));

        // stays enabled
        EXPECT_TRUE(config.getExposingOfChannelDataAsProperties());
    }

    TEST_F(AnAnimationNodeConfig, FailsToEnableDataExposeIfContainingElementsOfArrayType)
    {
        AnimationNodeConfig config;
        EXPECT_TRUE(config.addChannel({ "channel", m_dataFloat, m_dataVecVec }));

        EXPECT_FALSE(config.setExposingOfChannelDataAsProperties(true));
        EXPECT_FALSE(config.getExposingOfChannelDataAsProperties());
    }
}
