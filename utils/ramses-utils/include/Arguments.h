//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_ARGUMENTS_H
#define RAMSES_UTILS_ARGUMENTS_H

#include "Collections/String.h"

namespace ramses_internal
{
    class CommandLineParser;
}

enum EEffectIdType
{
    EEffectIdType_Client = 0,
    EEffectIdType_Renderer
};

class EffectConfig;

class Arguments
{
public:
    virtual ~Arguments() {}
    bool loadArguments(int argc, char const*const* argv);

    virtual bool parseArguments(int argc, char const*const* argv) = 0;
    virtual void printUsage() const = 0;

protected:
    static bool LoadMandatoryEffectConfig(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, EffectConfig& effectConfig);
    static bool LoadMandatoryExistingFileArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue);
    static bool LoadMandatoryArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue);
    static bool LoadOptionalArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue);

    static void PrintMissingParameter(const char* parameter);
    static void PrintMissingFile(const char* argmentName, const char* filePath);
};

#endif
