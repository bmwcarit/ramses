//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/InputStreamContainer.h"
#include "Components/FileInputStreamContainer.h"
#include "Components/MemoryInputStreamContainer.h"
#include "Components/OffsetFileInputStreamContainer.h"
#include "FileDescriptorHelper.h"
#include "gtest/gtest.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <array>

namespace ramses_internal
{
    TEST(AInputStreamContainer, canCreateAndUseWithFileInputStream)
    {
        const std::array<Byte, 5> dataW = {5, 4, 3, 2, 10};
        {
            File f("test.bin");
            EXPECT_TRUE(f.open(File::Mode::WriteNewBinary));
            EXPECT_TRUE(f.write(dataW.data(), dataW.size()));
        }

        FileInputStreamContainer is("test.bin");
        std::array<Byte, 5> dataR = {0};
        is.getStream().read(dataR.data(), dataR.size());
        EXPECT_EQ(dataW, dataR);
    }

    TEST(AInputStreamContainer, canCreateAndUseWithMemoryInputStream)
    {
        // static avoids need for capture in deleter and allows decay to function ptr
        static bool deleted = false;
        deleted = false;
        {
            const std::array<Byte, 5> dataW = {5, 4, 3, 2, 10};
            std::unique_ptr<Byte[], void(*)(const unsigned char*)> up(new Byte[dataW.size()],
                                                                      [](auto* ptr){
                                                                          deleted = true;
                                                                          delete[] ptr;
                                                                      });
            std::memcpy(up.get(), dataW.data(), dataW.size());

            MemoryInputStreamContainer is(std::move(up));
            std::array<Byte, 5> dataR = {0};
            is.getStream().read(dataR.data(), dataR.size());
            EXPECT_EQ(dataW, dataR);

            EXPECT_FALSE(deleted);
        }
        EXPECT_TRUE(deleted);
    }

    TEST(AInputStreamContainer, canCreateAndUseWithOffsetFileInputStream)
    {
        const std::array<Byte, 5> dataW = {5, 4, 3, 2, 10};
        {
            File f("test.bin");
            EXPECT_TRUE(f.open(File::Mode::WriteNewBinary));
            EXPECT_TRUE(f.write(dataW.data(), dataW.size()));
        }

        const int fd = FileDescriptorHelper::OpenFileDescriptorBinary("test.bin");
        OffsetFileInputStreamContainer is(fd, 1, 3);
        std::array<Byte, 3> dataR = {0};
        is.getStream().read(dataR.data(), dataR.size());

        std::array<Byte, 3> dataWsub{4, 3, 2};
        EXPECT_EQ(dataWsub, dataR);
    }
}
