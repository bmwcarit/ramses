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

bool FileUtils::WriteTextToFile(const ramses_internal::String& text, const char* filePath)
{
    ramses_internal::File file(filePath);
    if (!file.open(ramses_internal::File::Mode::WriteNew) ||
        !file.write(text.c_str(), text.size()))
        return false;
    file.flush();
    file.close();
    return true;
}

bool FileUtils::WriteHashToFile(ramses_internal::ResourceContentHash hash, const char* filePath)
{
    ramses_internal::File file(filePath);
    if (!file.open(ramses_internal::File::Mode::WriteNew))
        return false;

    const std::string content = fmt::format("0x{:016X}{:016X}\n", hash.highPart, hash.lowPart);
    if (!file.write(content.c_str(), content.size()))
        return false;
    file.flush();
    file.close();
    return true;
}

bool FileUtils::ReadFileLines(const char* filePath, std::vector<ramses_internal::String>& lines)
{
    ramses_internal::String contents;
    if (!ReadFileContentsToString(filePath, contents))
    {
        return false;
    }

    lines = ramses_internal::StringUtils::Tokenize(contents, '\n');
    return true;
}

bool FileUtils::ReadFileContentsToString(const char* filePath, ramses_internal::String& contents)
{
    ramses_internal::File file(filePath);
    if (!file.exists() ||
        !file.open(ramses_internal::File::Mode::ReadOnly))
    {
        return false;
    }

    ramses_internal::UInt fileSize = 0;
    ramses_internal::UInt readBytes = 0;
    if (!file.getSizeInBytes(fileSize))
    {
        PRINT_ERROR("fail to read from file: {}\n", filePath);
        return false;
    }

    std::vector<char> charVector(fileSize + 1u);
    const ramses_internal::EStatus stat = file.read(&charVector[0], fileSize, readBytes);
    if (stat == ramses_internal::EStatus::Ok || stat == ramses_internal::EStatus::Eof)
    {
        charVector[readBytes] = '\0';
        contents = &charVector[0];
        return true;
    }
    else
    {
        PRINT_ERROR("Fail to read file contents: {}", filePath);
        return false;
    }
}
