/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gmock/gmock.h"
#include "ramses-capu/util/FileUtils.h"
#include "ramses-capu/Config.h"
#include <vector>


TEST(FileUtilsTest, ReadWriteAllText)
{
    ramses_capu::File temp("temp.txt");
    EXPECT_FALSE(temp.exists());

    std::string content("this is some text\nwith multi lines and\ngerman umlauts öäüß !§$%&/()\r\nas well as some different line breaks and \\ escapes.");
    ramses_capu::FileUtils::writeAllText(temp, content);
    EXPECT_TRUE(temp.exists());

    std::string fromFile = ramses_capu::FileUtils::readAllText(temp);

    EXPECT_STREQ(content.c_str(), fromFile.c_str());

    std::string content2("this is a shorter text");
    ramses_capu::FileUtils::writeAllText(temp, content2);
    EXPECT_TRUE(temp.exists());

    std::string fromFile2 = ramses_capu::FileUtils::readAllText(temp);
    EXPECT_STREQ(content2.c_str(), fromFile2.c_str());

    temp.remove();
    EXPECT_FALSE(temp.exists());
}

TEST(FileUtilsTest, ReadWriteAllBytes)
{
    ramses_capu::File temp("readWriteBytesTest.dat");
    EXPECT_FALSE(temp.exists());

    ramses_capu::Byte largeContent[8000];

    for (uint32_t i = 0; i < sizeof(largeContent); i++)
    {
        largeContent[i] = i % 255;
    }

    ramses_capu::FileUtils::writeAllBytes(temp, largeContent, sizeof(largeContent));
    EXPECT_TRUE(temp.exists());

    std::vector<ramses_capu::Byte> readBuffer;
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::readAllBytes(temp, readBuffer));
    EXPECT_EQ(readBuffer.size(), sizeof(largeContent));
    EXPECT_EQ(0, ramses_capu::Memory::Compare(largeContent, readBuffer.data(), readBuffer.size()));

    // Keep the file and overwrite with a smaller set of data
    const ramses_capu::Byte smallContent[] = {0xAA, 0xBB, 0x0, 0xCC, 0xFF, 0x0, 0x05 };
    ramses_capu::FileUtils::writeAllBytes(temp, smallContent, sizeof(smallContent));
    readBuffer.clear(); // Re-use the same buffer
    EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::FileUtils::readAllBytes(temp, readBuffer));
    EXPECT_EQ(readBuffer.size(), sizeof(smallContent));
    EXPECT_EQ(0, ramses_capu::Memory::Compare(smallContent, readBuffer.data(), readBuffer.size()));

    temp.remove();
    EXPECT_FALSE(temp.exists());
}
