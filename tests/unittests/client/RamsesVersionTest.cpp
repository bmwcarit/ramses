//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RamsesVersion.h"
#include "gtest/gtest.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "ramses-sdk-build-config.h"

namespace ramses::internal
{
    class ARamsesVersion : public ::testing::Test
    {
    public:
        static BinaryOutputStream CreateOutputStreamFromString(const std::string& str)
        {
            BinaryOutputStream out;
            out.write(str.data(), static_cast<uint32_t>(str.size()));
            return out;
        }

        RamsesVersion::VersionInfo info;
        EFeatureLevel featureLevel = EFeatureLevel_01;
    };

    TEST_F(ARamsesVersion, canParseCurrentVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, info.versionString.c_str());
        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, info.gitHash.c_str());
        EXPECT_EQ(static_cast<uint32_t>(std::strtoul(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR, nullptr, 10)), info.major);
        EXPECT_EQ(static_cast<uint32_t>(std::strtoul(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR, nullptr, 10)), info.minor);
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_F(ARamsesVersion, canParseCurrentVersionFromFile)
    {
        {
            File fOut("ramsesVersionTest");
            BinaryFileOutputStream out(fOut);
            RamsesVersion::WriteToStream(out, ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, EFeatureLevel_Latest);
        }
        File fIn("ramsesVersionTest");
        BinaryFileInputStream in(fIn);
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_RAMSES_VERSION, info.versionString.c_str());
        EXPECT_STREQ(::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH, info.gitHash.c_str());
        EXPECT_EQ(static_cast<uint32_t>(std::strtoul(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR, nullptr, 10)), info.major);
        EXPECT_EQ(static_cast<uint32_t>(std::strtoul(::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR, nullptr, 10)), info.minor);
        EXPECT_EQ(EFeatureLevel_Latest, featureLevel);
    }

    TEST_F(ARamsesVersion, canParseReleaseVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "17.0.1", "af98afa8e94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("17.0.1", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(17u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseMasterVersion)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "0.0.0-devMaster", "af98afa8e94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("0.0.0-devMaster", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(0u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseOtherVersionSuffix)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "0.0.0-releaseCandidate", "af98afa8e94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("0.0.0-releaseCandidate", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(0u, info.major);
        EXPECT_EQ(0u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseVersionNumbersInCorrectOrder)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.2.3-devMaster", "af98afa8e94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.2.3-devMaster", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(2u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseLongVersionNumbers)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "11.22.333", "af98afa8e94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("11.22.333", info.versionString.c_str());
        EXPECT_STREQ("af98afa8e94", info.gitHash.c_str());
        EXPECT_EQ(11u, info.major);
        EXPECT_EQ(22u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseUnknownAsHashValue)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2", "(unknown)", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.3.2", info.versionString.c_str());
        EXPECT_STREQ("(unknown)", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseFullHashValue)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2", "ce01334641b90dbfc02cbee5d072cec8f9000afe", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.3.2", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
    }

    TEST_F(ARamsesVersion, canParseMajorMinorPatchTweak)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2.5", "ce01334641b90dbfc02cbee5d072cec8f9000afe", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.3.2.5", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseDifferentSuffix)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.2-foobar", "ce01334641b90dbfc02cbee5d072cec8f9000afe", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.3.2-foobar", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseRandomStringAfterMajorMinor)
    {
        BinaryOutputStream out;
        RamsesVersion::WriteToStream(out, "1.3.a.b.c+hotfix3", "ce01334641b90dbfc02cbee5d072cec8f9000afe", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));

        EXPECT_STREQ("1.3.a.b.c+hotfix3", info.versionString.c_str());
        EXPECT_STREQ("ce01334641b90dbfc02cbee5d072cec8f9000afe", info.gitHash.c_str());
        EXPECT_EQ(1u, info.major);
        EXPECT_EQ(3u, info.minor);
    }

    TEST_F(ARamsesVersion, canParseHandCraftedVersionInformation)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_TRUE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenHashEntryKeywordWrong)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHashX:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenHashEntryCharactersAreWrong)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d8XX9398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenHashMissing)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenVersionMissing)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenVersionTruncated)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenVersionNotNumbers)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.a.0]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenStartsWithZeroBytes)
    {
        const std::string str = "[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n";
        BinaryOutputStream out;
        uint32_t zeroData = 0;
        out.write(&zeroData, sizeof(zeroData));
        out.write(str.data(), str.size());

        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenStartsContainsZeroBytes)
    {
        const std::string str_1 = "[RamsesVersi";
        const std::string str_2 = "on:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n";
        BinaryOutputStream out;
        uint32_t zeroData = 0;
        out.write(str_1.data(), str_1.size());
        out.write(&zeroData, sizeof(zeroData));
        out.write(str_2.data(), str_2.size());

        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenStreamInvalid)
    {
        File f("this_file_should_not_exist");
        BinaryFileInputStream in(f);
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenNoNewlineFoundWithinReasonableDistanceFromStart)
    {
        std::string s;
        for (int i = 0; i < 1000; ++i)
        {
            s += ' ';
        }
        BinaryOutputStream out(CreateOutputStreamFromString(s));
        RamsesVersion::WriteToStream(out, "17.0.1", "af98afa8eeee94", EFeatureLevel_Latest);
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenStreamNotLongEnoughToMatchVersionIndicator)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersi\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenVersionNumberNegative)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:1.-1.0]\n[GitHash:d89398fd]\n[FeatureLevel:1]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelCompletelyMissing)
    {
        {
            // using actual file instead of byte stream because in this test EOF is reached
            // which is only handled properly when using file stream
            File f{ "tempFile" };
            ASSERT_TRUE(f.open(File::Mode::WriteNewBinary));
            const std::string data{ "[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n" };
            ASSERT_TRUE(f.write(data.data(), data.size()));
        }

        File f{ "tempFile" };
        ASSERT_TRUE(f.open(File::Mode::ReadOnlyBinary));
        BinaryFileInputStream stream{ f };
        EXPECT_FALSE(RamsesVersion::ReadFromStream(stream, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelLabelMismatch)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevelx:1]\n"));
        BinaryInputStream  in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelLabelBracketMissing)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:1\n"));
        BinaryInputStream  in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelLabelHasExtraChars)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:1].\n"));
        BinaryInputStream  in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelNotANumber)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:x]\n"));
        BinaryInputStream in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }

    TEST_F(ARamsesVersion, failsWhenFeatureLevelNotSupported)
    {
        BinaryOutputStream out(CreateOutputStreamFromString("[RamsesVersion:0.0.0]\n[GitHash:d89398fd]\n[FeatureLevel:99]\n"));
        BinaryInputStream  in(out.getData());
        EXPECT_FALSE(RamsesVersion::ReadFromStream(in, info, featureLevel));
    }
}
