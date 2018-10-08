//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Arguments.h"
#include "FileUtils.h"
#include "EffectConfig.h"
#include "ConsoleUtils.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"

void Arguments::PrintMissingParameter(const char* parameter)
{
    PRINT_ERROR("missing --%s\n", parameter);
}

void Arguments::PrintMissingFile(const char* argmentName, const char* filePath)
{
    PRINT_ERROR("file:\"%s\" for input argument %s does not exist!\n", filePath, argmentName);
}

bool Arguments::LoadMandatoryEffectConfig(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, EffectConfig& effectConfig)
{
    ramses_internal::String inEffectConfigStr;
    if (!LoadMandatoryExistingFileArgument(parser, argName, argShortName, inEffectConfigStr))
    {
        return false;
    }

    if (!effectConfig.loadFromFile(inEffectConfigStr.c_str()))
    {
        return false;
    }

    return true;
}

bool Arguments::LoadMandatoryExistingFileArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue)
{
    if (!LoadMandatoryArgument(parser, argName, argShortName, argValue))
    {
        return false;
    }

    if (!FileUtils::FileExists(argValue.c_str()))
    {
        PrintMissingFile(argName, argValue.c_str());
        return false;
    }

    return true;
}

bool Arguments::LoadMandatoryArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue)
{
    if (!LoadOptionalArgument(parser, argName, argShortName, argValue))
    {
        PrintMissingParameter(argName);
        return false;
    }

    return true;
}

bool Arguments::LoadOptionalArgument(const ramses_internal::CommandLineParser& parser, const char* argName, const char* argShortName, ramses_internal::String& argValue)
{
    argValue = ramses_internal::ArgumentString(parser, argShortName, argName, "");
    return argValue != ramses_internal::String();
}

bool Arguments::loadArguments(int argc, char const*const* argv)
{
    auto status = parseArguments(argc, argv);
    if (!status)
    {
        printUsage();
    }

    return status;
}
