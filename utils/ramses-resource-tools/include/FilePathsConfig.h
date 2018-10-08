//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCE_TOOLS_FILEPATHSCONFIG_H
#define RAMSES_RESOURCE_TOOLS_FILEPATHSCONFIG_H

#include "Collections/String.h"
#include "Collections/HashSet.h"

typedef ramses_internal::HashSet<ramses_internal::String> FilePaths;

class FilePathsConfig
{
public:
    bool loadFromFile(const char* filePath);
    const FilePaths& getFilePaths() const;

private:
    bool parseConfigLine(int lineNumber, const ramses_internal::String& line);
    void printErrorInLine(int lineNumber, const ramses_internal::String& line) const;

    FilePaths m_filePaths;
};

#endif
