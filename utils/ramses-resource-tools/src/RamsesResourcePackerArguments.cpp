//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesResourcePackerArguments.h"
#include "ConsoleUtils.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"

namespace
{
    const char* IN_RESOURCE_FILES_CONFIG_NAME = "in-resource-files-config";
    const char* IN_RESOURCE_FILES_CONFIG_SHORT_NAME = "ir";

    const char* OUT_RESOURCE_FILE_NAME = "out-resource-file";
    const char* OUT_RESOURCE_FILE_SHORT_NAME = "or";

    const char* OUT_COMPRESSION = "out-compression";
    const char* OUT_COMPRESSION_SHORT = "oc";
}

bool RamsesResourcePackerArguments::parseArguments(int argc, char const*const* argv)
{
    ramses_internal::CommandLineParser parser(argc, argv);
    if (!loadInputResourceFiles(parser))
    {
        return false;
    }

    if (!LoadMandatoryArgument(parser, OUT_RESOURCE_FILE_NAME, OUT_RESOURCE_FILE_SHORT_NAME, m_outputFile))
    {
        return false;
    }

    m_outCompression = ramses_internal::ArgumentBool(parser, OUT_COMPRESSION_SHORT, OUT_COMPRESSION);

    return true;
}

const ramses_internal::String& RamsesResourcePackerArguments::getOutputResourceFile() const
{
    return m_outputFile;
}

bool RamsesResourcePackerArguments::getUseCompression() const
{
    return m_outCompression;
}

void RamsesResourcePackerArguments::printUsage() const
{
    PRINT_HINT( "usage: program\n"
                "--{} (-{}) <filename>\n"
                "--{} (-{}) <filename>\n"
                "--{} (-{}) {{optional}}\n\n",
                IN_RESOURCE_FILES_CONFIG_NAME, IN_RESOURCE_FILES_CONFIG_SHORT_NAME,
                OUT_RESOURCE_FILE_NAME, OUT_RESOURCE_FILE_SHORT_NAME,
                OUT_COMPRESSION, OUT_COMPRESSION_SHORT);
}

bool RamsesResourcePackerArguments::loadInputResourceFiles(const ramses_internal::CommandLineParser& parser)
{
    ramses_internal::String inputResourceFilesConfig;
    if (!LoadMandatoryExistingFileArgument(parser, IN_RESOURCE_FILES_CONFIG_NAME, IN_RESOURCE_FILES_CONFIG_SHORT_NAME, inputResourceFilesConfig))
    {
        return false;
    }

    if (!m_inputFiles.loadFromFile(inputResourceFilesConfig.c_str()))
    {
        return false;
    }

    return true;
}

const FilePathsConfig& RamsesResourcePackerArguments::getInputResourceFiles() const
{
    return m_inputFiles;
}
