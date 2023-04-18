//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/StdFilesystemWrapper.h"

namespace rlogic
{
    // Works like pushd/popd with temp directory
    class WithTempDirectory
    {
    public:
        WithTempDirectory()
            : m_previousPath(fs::current_path())
        {
            fs::create_directory("sandbox");
            fs::current_path(m_previousPath / "sandbox");
        }

        ~WithTempDirectory()
        {
            fs::current_path(m_previousPath);
            fs::remove_all(m_previousPath / "sandbox");
        }

        WithTempDirectory(const WithTempDirectory&) = delete;
        WithTempDirectory& operator=(const WithTempDirectory&) = delete;

    private:
        fs::path m_previousPath;
    };

}
