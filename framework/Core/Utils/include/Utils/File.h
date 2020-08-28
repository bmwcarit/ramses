//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FILE_H
#define RAMSES_FILE_H

#include "Collections/String.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/Macros.h"
#include <cstdint>
#include <cstdio>

namespace ramses_internal
{
    class File final
    {
    public:
        enum class Mode
        {
            ReadOnly,                // opens file for reading
            WriteNew,                // opens file as an empty file for writing
            WriteExisting,           // opens file for reading and writing. The file must exist
            WriteOverWriteOld,       // opens file for reading and writing. Create a new file also of old one exists
            ReadOnlyBinary,          // opens file for reading in binary
            WriteNewBinary,          // opens file as an empty file for writing in binary
            WriteExistingBinary,     // opens file for reading and writing in binary. The file must exist
            WriteOverWriteOldBinary  // opens file for reading and writing in binary. Create a new file also of old one exists
        };

        enum class SeekOrigin
        {
            BeginningOfFile, // seeks from beginning of file
            RelativeToCurrentPosition // seeks relative to current position within the file
        };

        explicit File(String filepath);
        ~File();

        File(File&& other) noexcept;
        File& operator=(File&& other) noexcept;

        RNODISCARD bool exists() const;
        RNODISCARD bool isDirectory() const;
        bool createFile();
        bool createDirectory();
        bool remove();

        RNODISCARD bool open(const Mode& mode);
        bool close();
        RNODISCARD bool isOpen();
        RNODISCARD bool isEof();

        RNODISCARD bool getSizeInBytes(size_t& size) const;

        RNODISCARD EStatus read(void* buffer, size_t length, size_t& numBytes);
        RNODISCARD bool write(const void* buffer, size_t length);
        RNODISCARD bool seek(std::intptr_t numberOfBytesToSeek, SeekOrigin origin);
        RNODISCARD bool getPos(size_t& position) const;
        bool flush();

        RNODISCARD String getFileName() const;
        RNODISCARD String getPath() const;
        RNODISCARD String getExtension() const;

        File(const File&) = delete;
        File& operator=(File&) = delete;

    private:
        bool m_isOpen;
        String m_path;
        FILE* m_handle;
    };
}

#endif
