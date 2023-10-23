//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/text/FontRegistry.h"
#include "internal/Core/Utils/File.h"
#include "FileDescriptorHelper.h"
#include "gtest/gtest.h"

namespace ramses::internal
{
    class AFontRegistry : public testing::Test
    {
    public:
        AFontRegistry()
            : m_fontAccessor(m_fontRegistry)
        {
        }

    protected:
        FontRegistry m_fontRegistry;
        IFontAccessor& m_fontAccessor;
    };

    TEST_F(AFontRegistry, CreatesAndDestroysFreetype2FontInstanceFromFile)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        ASSERT_TRUE(fontInstanceId.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
    }

    TEST_F(AFontRegistry, CreatesAndDestroysFreetype2FontInstanceFromFileDescriptor)
    {
        const char* path = "./res/ramses-text-Roboto-Bold.ttf";
        const int fd = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary(path);
        size_t fileSize = 0;
        EXPECT_TRUE(ramses::internal::File(path).getSizeInBytes(fileSize));

        const FontId fontId = m_fontRegistry.createFreetype2FontFromFileDescriptor(fd, 0, fileSize);
        ASSERT_TRUE(fontId.isValid());

        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        ASSERT_TRUE(fontInstanceId.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
    }

    TEST_F(AFontRegistry, CreatesAndDestroysFreetype2FontInstanceFromFileDescriptorWithOffset)
    {
        size_t fileSize = 0;
        {
            // write to a file with some offset
            ramses::internal::File inFile("./res/ramses-text-Roboto-Bold.ttf");
            EXPECT_TRUE(inFile.getSizeInBytes(fileSize));
            std::vector<unsigned char> data(fileSize);
            size_t numBytesRead = 0;
            EXPECT_TRUE(inFile.open(ramses::internal::File::Mode::ReadOnlyBinary));
            EXPECT_EQ(ramses::internal::EStatus::Ok, inFile.read(data.data(), fileSize, numBytesRead));

            ramses::internal::File outFile("fontFileWithOffset.ttf");
            EXPECT_TRUE(outFile.open(ramses::internal::File::Mode::WriteOverWriteOldBinary));

            uint32_t zeroData = 0;
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
            EXPECT_TRUE(outFile.write(data.data(), data.size()));
            EXPECT_TRUE(outFile.write(&zeroData, sizeof(zeroData)));
        }

        const int fd = ramses::internal::FileDescriptorHelper::OpenFileDescriptorBinary("fontFileWithOffset.ttf");
        const FontId fontId = m_fontRegistry.createFreetype2FontFromFileDescriptor(fd, 4, fileSize);
        ASSERT_TRUE(fontId.isValid());

        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        ASSERT_TRUE(fontInstanceId.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
    }

    TEST_F(AFontRegistry, CreatesAndDestroysMultipleFreetype2FontInstancesFromFile)
    {
        const FontId fontId1 = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        const FontId fontId2 = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId1.isValid());
        ASSERT_TRUE(fontId2.isValid());
        const FontInstanceId fontInstanceId1 = m_fontRegistry.createFreetype2FontInstance(fontId1, 12u);
        const FontInstanceId fontInstanceId2 = m_fontRegistry.createFreetype2FontInstance(fontId2, 14u);
        ASSERT_TRUE(fontInstanceId1.isValid());
        ASSERT_TRUE(fontInstanceId2.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId2));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId2));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId1));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId2));
    }

    TEST_F(AFontRegistry, ReturnsNULLFontInstanceIfNotRegistered)
    {
        EXPECT_TRUE(nullptr == m_fontAccessor.getFontInstance(FontInstanceId(15u)));
    }

    TEST_F(AFontRegistry, FailsToCreateFontFromWrongPath)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/i_dont_exist.ttf");
        EXPECT_FALSE(fontId.isValid());
    }

    TEST_F(AFontRegistry, FailsToCreateFontFromWrongFileDescriptor)
    {
        const FontId fontId = m_fontRegistry.createFreetype2FontFromFileDescriptor(-1, 0, 10);
        EXPECT_FALSE(fontId.isValid());
    }

    TEST_F(AFontRegistry, FailsToCreateFontFromWrongFileDescriptorLength)
    {
        const FontId fontId = m_fontRegistry.createFreetype2FontFromFileDescriptor(1, 0, 0);
        EXPECT_FALSE(fontId.isValid());
    }

    TEST_F(AFontRegistry, FailsToCreateFontInstanceFromInvalidFont)
    {
        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(FontId(15u), 12u);
        EXPECT_FALSE(fontInstanceId.isValid());
    }

    TEST_F(AFontRegistry, FailsToCreateFontInstanceFromDeletedFont)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        m_fontRegistry.deleteFont(fontId);

        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        EXPECT_FALSE(fontInstanceId.isValid());
    }

    TEST_F(AFontRegistry, FailsToDestroyInvalidFont)
    {
        EXPECT_FALSE(m_fontRegistry.deleteFont(FontId(15u)));
    }

    TEST_F(AFontRegistry, FailsToDestroyInvalidFontInstance)
    {
        EXPECT_FALSE(m_fontRegistry.deleteFontInstance(FontInstanceId(15u)));
    }

    TEST_F(AFontRegistry, InstantiatesSecondFontRegistryAndCreatesAnotherFontInstance)
    {
        // Main font registry
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        ASSERT_TRUE(fontInstanceId.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId));

        // Other font registry
        FontRegistry otherFontRegistry;
        const FontId fontId2 = otherFontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId2.isValid());
        const FontInstanceId fontInstanceId2 = otherFontRegistry.createFreetype2FontInstance(fontId2, 12u);
        ASSERT_TRUE(fontInstanceId2.isValid());
        EXPECT_TRUE(otherFontRegistry.getFontInstance(fontInstanceId2));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
        EXPECT_TRUE(otherFontRegistry.deleteFontInstance(fontInstanceId2));
        EXPECT_TRUE(otherFontRegistry.deleteFont(fontId2));
    }

    // Harfbuzz
    TEST_F(AFontRegistry, CreatesAndDestroysHBFontInstanceFromFile)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(fontId, 12u);
        ASSERT_TRUE(fontInstanceId.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
    }

    TEST_F(AFontRegistry, CreatesAndDestroysMultipleHBFontInstancesFromFile)
    {
        const FontId fontId1 = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        const FontId fontId2 = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId1.isValid());
        ASSERT_TRUE(fontId2.isValid());
        const FontInstanceId fontInstanceId1 = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(fontId1, 12u);
        const FontInstanceId fontInstanceId2 = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(fontId2, 14u);
        ASSERT_TRUE(fontInstanceId1.isValid());
        ASSERT_TRUE(fontInstanceId2.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId2));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId2));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId1));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId2));
    }

    TEST_F(AFontRegistry, CreatesAndDestroysBothFT2AndHBFontInstancesFromFile)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        const FontInstanceId fontInstanceId1 = m_fontRegistry.createFreetype2FontInstance(fontId, 12u);
        const FontInstanceId fontInstanceId2 = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(fontId, 14u);
        ASSERT_TRUE(fontInstanceId1.isValid());
        ASSERT_TRUE(fontInstanceId2.isValid());
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontAccessor.getFontInstance(fontInstanceId2));

        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId1));
        EXPECT_TRUE(m_fontRegistry.deleteFontInstance(fontInstanceId2));
        EXPECT_TRUE(m_fontRegistry.deleteFont(fontId));
    }

    TEST_F(AFontRegistry, FailsToCreateHBFontInstanceFromInvalidFont)
    {
        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(FontId(15u), 12u);
        EXPECT_FALSE(fontInstanceId.isValid());
    }

    TEST_F(AFontRegistry, FailsToCreateHBFontInstanceFromDeletedFont)
    {
        const FontId fontId = m_fontRegistry.createFreetype2Font("./res/ramses-text-Roboto-Bold.ttf");
        ASSERT_TRUE(fontId.isValid());
        m_fontRegistry.deleteFont(fontId);

        const FontInstanceId fontInstanceId = m_fontRegistry.createFreetype2FontInstanceWithHarfBuzz(fontId, 12u);
        EXPECT_FALSE(fontInstanceId.isValid());
    }
}
