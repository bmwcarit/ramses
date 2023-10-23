//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformError.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <string>

#include <fmt/format.h>

namespace ramses::internal
{
    class File final
    {
    public:
        enum class Mode
        {
            ReadOnly,                // opens file for reading
            WriteNew,                // opens file as an empty file for writing
            WriteExisting,           // opens file for reading and writing. The file must exist
            WriteOverWriteOld,       // opens file for reading and writing. Create a new file also of old one exists
            ReadOnlyBinary,          // opens file for reading in binary
            WriteNewBinary,          // opens file as an empty file for writing in binary
            WriteExistingBinary,     // opens file for reading and writing in binary. The file must exist
            WriteOverWriteOldBinary  // opens file for reading and writing in binary. Create a new file also of old one exists
        };

        enum class SeekOrigin
        {
            BeginningOfFile, // seeks from beginning of file
            RelativeToCurrentPosition // seeks relative to current position within the file
        };

        static constexpr std::array SeekOriginNames
        {
            "BeginningOfFile",
            "RelativeToCurrentPosition",
        };

        explicit File(std::string_view filepath);
        ~File();

        File(File&& other) noexcept;
        File& operator=(File&& other) noexcept;

        [[nodiscard]] bool exists() const;
        [[nodiscard]] bool isDirectory() const;
        bool createFile();
        bool createDirectory();
        bool remove();

        [[nodiscard]] bool open(const Mode& mode);
        bool close();
        [[nodiscard]] bool isOpen() const;
        [[nodiscard]] bool isEof() const;

        [[nodiscard]] bool getSizeInBytes(size_t& size) const;

        [[nodiscard]] EStatus read(void* buffer, size_t length, size_t& numBytes);
        [[nodiscard]] bool write(const void* buffer, size_t length);
        [[nodiscard]] bool seek(int64_t offset, SeekOrigin origin);
        [[nodiscard]] bool getPos(size_t& position) const;
        bool flush();

        [[nodiscard]] std::string getFileName() const;
        [[nodiscard]] std::string getPath() const;
        [[nodiscard]] std::string getExtension() const;

        File(const File&) = delete;
        File& operator=(File&) = delete;

    private:
        bool m_isOpen{false};
        std::filesystem::path m_path{};
        std::FILE* m_handle{nullptr};
    };
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::File::SeekOrigin,
                                        "SeekOrigin",
                                        ramses::internal::File::SeekOriginNames,
                                        ramses::internal::File::SeekOrigin::RelativeToCurrentPosition);
