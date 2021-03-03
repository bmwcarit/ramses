//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneAPI/TextureSamplerStates.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    class TextureSamplerStatesHash : public ::testing::Test
    {
    protected:
        void expectSameHash(const TextureSamplerStates& state1, const TextureSamplerStates& state2)
        {
            const auto hash1 = state1.hash();
            const auto hash2 = state2.hash();
            EXPECT_EQ(hash1, hash2);
        }

        void expectNotSameHash(const TextureSamplerStates& state1, const TextureSamplerStates& state2)
        {
            const auto hash1 = state1.hash();
            const auto hash2 = state2.hash();
            EXPECT_NE(hash1, hash2);
        }
    };

    TEST_F(TextureSamplerStatesHash, SameStatesHaveSameHash)
    {
        {
            const TextureSamplerStates samplerStates;
            const TextureSamplerStates otherSamplerStates;
            expectSameHash(samplerStates, otherSamplerStates);
        }

        {
            const TextureSamplerStates samplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            expectSameHash(samplerStates, otherSamplerStates);
        }
    }

    TEST_F(TextureSamplerStatesHash, DifferentStatesHaveDifferentHash)
    {
        const TextureSamplerStates samplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);

        //address mode U
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Repeat, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
        //address mode V
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::RepeatMirrored, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
        //address mode R
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::Clamp, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
        //min sampling
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Nearest_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 8u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
        //mag sampling
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Linear, 8u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
        //anisotropy level
        {
            const TextureSamplerStates otherSamplerStates(EWrapMethod::Clamp, EWrapMethod::Repeat, EWrapMethod::RepeatMirrored, ESamplingMethod::Linear_MipMapLinear, ESamplingMethod::Nearest_MipMapNearest, 1u);
            expectNotSameHash(samplerStates, otherSamplerStates);
        }
    }
}
