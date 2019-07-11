//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FILE_H
#define RAMSES_FILE_H

#include <ramses-capu/os/File.h>

#include <PlatformAbstraction/PlatformTypes.h>
#include <PlatformAbstraction/PlatformError.h>
#include "Collections/String.h"

namespace ramses_internal
{
    enum EFileMode
    {
        EFileMode_ReadOnly = ramses_capu::READ_ONLY,                // opens file for reading
        EFileMode_WriteNew = ramses_capu::WRITE_NEW,                // opens file as an empty file for writing
        EFileMode_WriteExisting = ramses_capu::READ_WRITE_EXISTING,      // opens file for reading and writing. The file must exist
        EFileMode_WriteOverWriteOld = ramses_capu::READ_WRITE_OVERWRITE_OLD, // opens file for reading and writing. Create a new file also of old one exists
        EFileMode_ReadOnlyBinary = ramses_capu::READ_ONLY_BINARY,         // opens file for reading in binary
        EFileMode_WriteNewBinary = ramses_capu::WRITE_NEW_BINARY,         // opens file as an empty file for writing in binary
        EFileMode_WriteExistingBinary = ramses_capu::READ_WRITE_EXISTING_BINARY,      // opens file for reading and writing in binary. The file must exist
        EFileMode_WriteOverWriteOldBinary = ramses_capu::READ_WRITE_OVERWRITE_OLD_BINARY // opens file for reading and writing in binary. Create a new file also of old one exists
    };

    enum EFileSeekOrigin
    {
        EFileSeekOrigin_BeginningOfFile = ramses_capu::FROM_BEGINNING, // seeks from beginning of file
        EFileSeekOrigin_RelativeToCurrentPosition = ramses_capu::FROM_CURRENT_POSITION // seeks relative to current position within the file
    };

    class File: private ramses_capu::File
    {
    public:
        explicit File(const String& filepath);
        EStatus open(const EFileMode& mode);
        EStatus createFile();
        EStatus createDirectory();
        Bool exists() const;
        EStatus remove();
        Bool isOpen();
        Bool isEof();
        Bool isDirectory() const;
        EStatus getSizeInBytes(UInt& size) const;
        EStatus read(Char* buffer, UInt length, UInt& numBytes);
        EStatus write(const Char* buffer, UInt length);
        EStatus seek(Int numberOfBytesToSeek, EFileSeekOrigin origin);
        EStatus getPos(UInt& position);
        EStatus flush();
        EStatus close();
        String getFileName() const;
        String getPath() const;
        String getExtension() const;

    private:
        explicit File(const ramses_capu::File& file);

        friend class BinaryFileOutputStream;
        friend class BinaryFileInputStream;
    };

    inline
    File::File(const String& filepath)
        : ramses_capu::File(filepath.stdRef())
    {
    }

    inline
    File::File(const ramses_capu::File& file)
        : ramses_capu::File(file.getPath())
    {
    }

    inline
    EStatus File::open(const EFileMode& mode)
    {
        return static_cast<EStatus>(ramses_capu::File::open(ramses_capu::FileMode(mode)));
    }

    inline
    EStatus
    File::createFile()
    {
        return static_cast<EStatus>(ramses_capu::File::createFile());
    }

    inline
    EStatus
    File::createDirectory()
    {
        return static_cast<EStatus>(ramses_capu::File::createDirectory());
    }

    inline
    Bool
    File::exists() const
    {
        return ramses_capu::File::exists();
    }

    inline
    EStatus
    File::remove()
    {
        return static_cast<EStatus>(ramses_capu::File::remove());
    }

    inline
    Bool
    File::isOpen()
    {
        return ramses_capu::File::isOpen();
    }

    inline
    Bool
    File::isEof()
    {
        return ramses_capu::File::isEof();
    }

    inline
    Bool
    File::isDirectory() const
    {
        return ramses_capu::File::isDirectory();
    }

    inline
    EStatus
    File::getSizeInBytes(UInt& size) const
    {
        return static_cast<EStatus>(ramses_capu::File::getSizeInBytes(size));
    }

    inline
    EStatus
    File::read(Char* buffer, UInt length, UInt& numBytes)
    {
        return static_cast<EStatus>(ramses_capu::File::read(buffer, length, numBytes));
    }

    inline
    EStatus
    File::write(const Char* buffer, UInt length)
    {
        return static_cast<EStatus>(ramses_capu::File::write(buffer, length));
    }

    inline
    EStatus
    File::flush()
    {
        return static_cast<EStatus>(ramses_capu::File::flush());
    }

    inline
    EStatus
    File::close()
    {
        return static_cast<EStatus>(ramses_capu::File::close());
    }

    inline
    String
    File::getFileName() const
    {
        return String(ramses_capu::File::getFileName());
    }

    inline
    String
    File::getPath() const
    {
        return String(ramses_capu::File::getPath());
    }

    inline
    String
    File::getExtension() const
    {
        return String(ramses_capu::File::getExtension());
    }

    inline
    ramses_internal::EStatus File::seek(Int numberOfBytesToSeek, EFileSeekOrigin origin)
    {
        return static_cast<EStatus>(ramses_capu::File::seek(numberOfBytesToSeek, static_cast<ramses_capu::FileSeekOrigin>(origin)));
    }

    inline
    ramses_internal::EStatus File::getPos(UInt& position)
    {
        return static_cast<EStatus>(ramses_capu::File::getCurrentPosition(position));
    }

}

#endif
