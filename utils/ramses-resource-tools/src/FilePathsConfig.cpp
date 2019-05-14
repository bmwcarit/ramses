//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FilePathsConfig.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "ConsoleUtils.h"

bool FilePathsConfig::loadFromFile(const char* filePath)
{
    m_filePaths.clear();

    if (!FileUtils::FileExists(filePath))
    {
        return false;
    }

    std::vector<ramses_internal::String> lines;
    if (!FileUtils::ReadFileLines(filePath, lines))
    {
        return false;
    }

    bool succeed = true;
    int lineNumber = 1;
    for(const auto& line : lines)
    {
        if (!parseConfigLine(lineNumber++, line))
        {
            succeed = false;
        }
    }

    if (!succeed)
    {
        m_filePaths.clear();
    }

    return succeed;
}

const FilePaths& FilePathsConfig::getFilePaths() const
{
    return m_filePaths;
}

bool FilePathsConfig::parseConfigLine(int lineNumber, const ramses_internal::String& line)
{
    if (0 == line.getLength())
    {
        return true;
    }

    std::vector<ramses_internal::String> tokens;
    StringUtils::GetLineTokens(line, ' ', tokens);
    if (tokens.empty())
    {
        return true;
    }

    if (1 != tokens.size())
    {
        printErrorInLine(lineNumber, line);
        PRINT_HINT("expecting a single file path per line in file paths config file.\n\n");
        return false;
    }

    const ramses_internal::String& filePath = tokens[0];
    if (!FileUtils::FileExists(filePath.c_str()))
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("file with path \"%s\" does not exist.\n\n", filePath.c_str());
        return false;
    }

    if (m_filePaths.hasElement(filePath))
    {
        printErrorInLine(lineNumber, line);
        PRINT_INFO("file with path \"%s\" is duplicate in config file.\n\n", filePath.c_str());
        return false;
    }

    m_filePaths.put(filePath);
    return true;
}

void FilePathsConfig::printErrorInLine(int lineNumber, const ramses_internal::String& line) const
{
    PRINT_ERROR("file paths config file error in line : #%d, \"%s\"\n", lineNumber, line.c_str());
}
