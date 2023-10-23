//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/SceneAPI/TextureSamplerStates.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class TextureSamplerStatesHash : public ::testing::Test
    {
    protected:
        static void ExpectSameHash(const TextureSamplerStates& state1, const TextureSamplerStates& state2)
        {
            const auto hash1 = state1.hash();
            const auto hash2 = state2.hash();
            EXPECT_EQ(hash1, hash2);
        }

        static void ExpectNotSameHash(const TextureSamplerStates& state1, const TextureSamplerStates& state2)
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
            ExpectSameHash(samplerStates, otherSamplerStates);
        }

        {
            const TextureSamplerStates samplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            ExpectSameHash(samplerStates, otherSamplerStates);
        }
    }

    TEST_F(TextureSamplerStatesHash, DifferentStatesHaveDifferentHash)
    {
        const TextureSamplerStates samplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);

        //address mode U
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Repeat, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
        //address mode V
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Mirror, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
        //address mode R
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Clamp, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
        //min sampling
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Nearest_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 8u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
        //mag sampling
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Linear, 8u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
        //anisotropy level
        {
            const TextureSamplerStates otherSamplerStates(ETextureAddressMode::Clamp, ETextureAddressMode::Repeat, ETextureAddressMode::Mirror, ETextureSamplingMethod::Linear_MipMapLinear, ETextureSamplingMethod::Nearest_MipMapNearest, 1u);
            ExpectNotSameHash(samplerStates, otherSamplerStates);
        }
    }
}
