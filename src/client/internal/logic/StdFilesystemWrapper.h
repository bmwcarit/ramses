//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#if defined(RLOGIC_STD_FILESYSTEM_EXPERIMENTAL)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#elif defined(RLOGIC_STD_FILESYSTEM_EMULATION)

// add more emulation methods when needed
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs
{
    bool is_directory(const std::string& path)
    {
        struct stat tmp;
        return stat(path.c_str(), &tmp) == 0 && S_ISDIR(tmp.st_mode);
    }
}

#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

