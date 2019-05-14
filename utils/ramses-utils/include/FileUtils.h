//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_UTILS_FILEUTILS_H
#define RAMSES_UTILS_FILEUTILS_H

#include "Collections/String.h"
#include "Collections/Vector.h"
#include "SceneAPI/ResourceContentHash.h"

class FileUtils
{
public:
    static bool FileExists(const char* filePath);
    static ramses_internal::String GetFileName(const char* filePath);

    static void RemoveFileIfExist(const char* filePath);

    static void WriteTextToFile(const ramses_internal::String& text, const char* filePath);
    static void WriteHashToFile(ramses_internal::ResourceContentHash hash, const char* filePath);

    static bool ReadFileLines(const char* filePath, std::vector<ramses_internal::String>& lines);
    static bool ReadFileContentsToString(const char* filePath, ramses_internal::String& contents);
};

#endif
