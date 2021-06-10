//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTUTILS_FILEDESCRIPTORHELPER_H
#define RAMSES_TESTUTILS_FILEDESCRIPTORHELPER_H

#include <fcntl.h>
#if _WIN32
#  include <io.h>
#else
#  include <unistd.h>
#endif

namespace ramses_internal
{
    namespace FileDescriptorHelper
    {
        inline int OpenFileDescriptorBinary(const char* path, int flags = O_RDONLY)
        {
#if _WIN32
            return ::open(path, flags | O_BINARY);
#else
            return ::open(path, flags);
#endif
        }
    }
}

#endif
