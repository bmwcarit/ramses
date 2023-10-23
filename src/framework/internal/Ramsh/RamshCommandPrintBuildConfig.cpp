//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Ramsh/RamshCommandPrintBuildConfig.h"
#include "internal/Ramsh/Ramsh.h"
#include "internal/Core/Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"

namespace ramses::internal
{
    RamshCommandPrintBuildConfig::RamshCommandPrintBuildConfig()
    {
        registerKeyword("buildConfig");
        description = "print build configuration";
    }

    bool RamshCommandPrintBuildConfig::executeInput(const std::vector<std::string>& /*unused*/)
    {
        LOG_INFO(CONTEXT_RAMSH,
            "VERSION_STRING = "     << ::ramses_sdk::RAMSES_SDK_RAMSES_VERSION << "\n" <<
            "GIT_COMMIT_COUNT = "   << ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_COUNT << "\n" <<
            "GIT_COMMIT_HASH = "    << ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH << "\n" <<
            "BUILD_TYPE = "         << ::ramses_sdk::RAMSES_SDK_CMAKE_BUILD_TYPE << "\n" <<
            "CXX_COMPILER = "       << ::ramses_sdk::RAMSES_SDK_CMAKE_CXX_COMPILER << "\n" <<
            "CXX_COMPILER_ID = "    << ::ramses_sdk::RAMSES_SDK_CMAKE_CXX_COMPILER_ID << "\n" <<
            "CXX_FLAGS = "          << ::ramses_sdk::RAMSES_SDK_CMAKE_CXX_FLAGS << "\n" <<
            "CXX_FLAGS_DEBUG = "    << ::ramses_sdk::RAMSES_SDK_CMAKE_CXX_FLAGS_DEBUG << "\n" <<
            "CXX_FLAGS_RELEASE = "  << ::ramses_sdk::RAMSES_SDK_CMAKE_CXX_FLAGS_RELEASE << "\n" <<
            "BUILD_SYSTEM_NAME = "  << ::ramses_sdk::RAMSES_SDK_CMAKE_SYSTEM_NAME << "\n" <<
            "CMAKE_VERSION = "      << ::ramses_sdk::RAMSES_SDK_CMAKE_VERSION << "\n" <<
            "BUILD_ENV_INFO = "     << ::ramses_sdk::RAMSES_SDK_BUILD_ENV_VERSION_INFO_FULL << "\n"
            );
        return true;
    }
}
