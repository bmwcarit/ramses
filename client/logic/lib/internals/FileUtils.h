//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>
#include <optional>
#include <vector>

namespace rlogic::internal
{
    class FileUtils
    {
    public:
        static bool SaveBinary(const std::string& filename, const void* binaryBuffer, size_t bufferLength);
        static std::optional<std::vector<char>> LoadBinary(const std::string& filename);
        static std::optional<std::vector<char>> LoadBinary(int fd, size_t offset, size_t size);
    };
}
