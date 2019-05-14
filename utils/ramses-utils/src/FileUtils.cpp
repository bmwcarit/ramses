//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "FileUtils.h"
#include "ConsoleUtils.h"

#include "Collections/StringOutputStream.h"
#include "Utils/File.h"
#include "Utils/StringUtils.h"

bool FileUtils::FileExists(const char* filePath)
{
    ramses_internal::File file(filePath);
    return file.exists();
}

ramses_internal::String FileUtils::GetFileName(const char* filePath)
{
    ramses_internal::File file(filePath);
    return file.getFileName();
}

void FileUtils::RemoveFileIfExist(const char* filePath)
{
    ramses_internal::File file(filePath);
    if (file.exists())
    {
        file.remove();
    }
}

void FileUtils::WriteTextToFile(const ramses_internal::String& text, const char* filePath)
{
    ramses_internal::File file(filePath);
    file.open(ramses_internal::EFileMode_WriteNew);

    file.write(text.c_str(), text.getLength());
    file.flush();

    file.close();
}

void FileUtils::WriteHashToFile(ramses_internal::ResourceContentHash hash, const char* filePath)
{
    ramses_internal::File file(filePath);
    file.open(ramses_internal::EFileMode_WriteNew);

    ramses_internal::StringOutputStream stringStream;
    stringStream << hash << "\n";
    file.write(stringStream.c_str(), stringStream.length());
    file.flush();

    file.close();
}

bool FileUtils::ReadFileLines(const char* filePath, std::vector<ramses_internal::String>& lines)
{
    lines.clear();

    ramses_internal::String contents;
    if (!ReadFileContentsToString(filePath, contents))
    {
        return false;
    }

    ramses_internal::StringUtils::Tokenize(contents, lines, '\n');
    return true;
}

bool FileUtils::ReadFileContentsToString(const char* filePath, ramses_internal::String& contents)
{
    ramses_internal::File file(filePath);
    if (!file.exists())
    {
        return false;
    }

    file.open(ramses_internal::EFileMode_ReadOnly);

    ramses_internal::UInt fileSize = 0;
    ramses_internal::UInt readBytes = 0;
    ramses_internal::EStatus stat = file.getSizeInBytes(fileSize);
    if (stat != ramses_internal::EStatus_RAMSES_OK)
    {
        PRINT_ERROR("fail to read from file: %s\n", filePath);
        return false;
    }

    std::vector<char> charVector(fileSize + 1u);
    stat = file.read(&charVector[0], fileSize, readBytes);
    if (stat == ramses_internal::EStatus_RAMSES_OK || stat == ramses_internal::EStatus_RAMSES_EOF)
    {
        charVector[readBytes] = '\0';
        contents = &charVector[0];
        return true;
    }
    else
    {
        PRINT_ERROR("Fail to read file contents: %s", filePath);
        return false;
    }
}
