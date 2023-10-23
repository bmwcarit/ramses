//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/InputStreamContainer.h"
#include "internal/Components/FileInputStreamContainer.h"
#include "internal/Components/MemoryInputStreamContainer.h"
#include "internal/Components/OffsetFileInputStreamContainer.h"
#include "FileDescriptorHelper.h"
#include "gtest/gtest.h"
#include <memory>
#include <sys/stat.h>
#include <fcntl.h>
#include <array>

namespace ramses::internal
{
    template <typename... Ts>
    std::array<std::byte, sizeof...(Ts)> make_byte_array(Ts&&... args) noexcept
    {
        return {std::byte(std::forward<Ts>(args))...};
    }

    TEST(AInputStreamContainer, canCreateAndUseWithFileInputStream)
    {
        const std::array<std::byte, 5> dataW = make_byte_array(5, 4, 3, 2, 10);
        {
            File f("test.bin");
            EXPECT_TRUE(f.open(File::Mode::WriteNewBinary));
            EXPECT_TRUE(f.write(dataW.data(), dataW.size()));
        }

        FileInputStreamContainer is("test.bin");
        std::array<std::byte, 5> dataR = {std::byte{0}};
        is.getStream().read(dataR.data(), dataR.size());
        EXPECT_EQ(dataW, dataR);
    }

    TEST(AInputStreamContainer, canCreateAndUseWithMemoryInputStream)
    {
        // static avoids need for capture in deleter and allows decay to function ptr
        static bool deleted = false;
        deleted = false;
        {
            const std::array<std::byte, 5> dataW = make_byte_array(5, 4, 3, 2, 10);
            std::unique_ptr<std::byte[], void(*)(const std::byte*)> up(new std::byte[dataW.size()],
                                                                      [](auto* ptr){
                                                                          deleted = true;
                                                                          delete[] ptr;
                                                                      });
            std::memcpy(up.get(), dataW.data(), dataW.size());

            MemoryInputStreamContainer is(std::move(up));
            std::array<std::byte, 5> dataR = {std::byte{0}};
            is.getStream().read(dataR.data(), dataR.size());
            EXPECT_EQ(dataW, dataR);

            EXPECT_FALSE(deleted);
        }
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) gets confused by custom delter
        EXPECT_TRUE(deleted);
    }

    TEST(AInputStreamContainer, canCreateAndUseWithOffsetFileInputStream)
    {
        const std::array<std::byte, 5> dataW = make_byte_array(5, 4, 3, 2, 10);
        {
            File f("test.bin");
            EXPECT_TRUE(f.open(File::Mode::WriteNewBinary));
            EXPECT_TRUE(f.write(dataW.data(), dataW.size()));
        }

        const int fd = FileDescriptorHelper::OpenFileDescriptorBinary("test.bin");
        OffsetFileInputStreamContainer is(fd, 1, 3);
        std::array<std::byte, 3> dataR = {std::byte{0}};
        is.getStream().read(dataR.data(), dataR.size());

        std::array<std::byte, 3> dataWsub = make_byte_array(4, 3, 2);
        EXPECT_EQ(dataWsub, dataR);
    }
}
