//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FILEUTILS_H
#define RAMSES_FILEUTILS_H

#include <ramses-capu/util/FileUtils.h>
#include "Utils/File.h"
#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    class FileUtils
    {
    public:
        /**
        * Reads all text from a file.
        * @param file The file containing the text.
        * @return The file content as string.
        */
        static String ReadAllText(File& file);

        /**
        * Writes all given text in a file. Existing content will get overwritten.
        * @param file The file into which to content will get written.
        * @param content The content that should go into the file.
        * @return The return value.
        */
        static EStatus WriteAllText(File& file, const String& content);

        /**
        * Writes all bytes to a file. Existing content will be overwritten.
        * @param file The file which the content will get written to.
        * @param buffer A pointer to an input buffer with byte data.
        * @param numberOfBytesToWrite The number of bytes to write.
        * @return The return value.
        */
        static EStatus WriteAllBytes(File& file, const Byte* buffer, UInt32 numberOfBytesToWrite);

        /**
        * Reads all bytes from a file.
        * @param file The file containing the bytes.
        * @param result A vector which the resulting bytes will be appended to.
        * @return The return value.
        */
        static EStatus ReadAllBytes(File& file, std::vector<Byte>& result);
    };

    inline String FileUtils::ReadAllText(File& file)
    {
        ramses_capu::File capuFile = ramses_capu::File(file.getPath().stdRef());
        return String(ramses_capu::FileUtils::readAllText(capuFile));
    }

    inline EStatus FileUtils::WriteAllText(File& file, const String& content)
    {
        ramses_capu::File capuFile = ramses_capu::File(file.getPath().stdRef());
        return static_cast<EStatus>(ramses_capu::FileUtils::writeAllText(capuFile,content.stdRef()));
    }

    inline EStatus FileUtils::WriteAllBytes(File& file, const Byte* buffer, UInt32 numberOfBytesToWrite)
    {
        ramses_capu::File capuFile = ramses_capu::File(file.getPath().stdRef());
        return static_cast<EStatus>(ramses_capu::FileUtils::writeAllBytes(capuFile, buffer, numberOfBytesToWrite));
    }

    inline EStatus FileUtils::ReadAllBytes(File& file, std::vector<Byte>& result)
    {
        ramses_capu::File capuFile = ramses_capu::File(file.getPath().stdRef());
        ramses_capu::uint_t fileSize;
        if (capuFile.getSizeInBytes(fileSize) != ramses_capu::CAPU_OK)
        {
            // error determining file size
            return static_cast<EStatus>(ramses_capu::CAPU_ERROR);
        }

        result.resize(fileSize);
        capuFile.open(ramses_capu::READ_ONLY_BINARY);
        ramses_capu::uint_t totalBytesRead = 0;
        ramses_capu::status_t retVal = ramses_capu::CAPU_ERROR;

        // Read directly into the vector
        char* vectorPtr = reinterpret_cast<char*>(result.data());

        while (totalBytesRead < fileSize)
        {
            ramses_capu::uint_t bytesRead = 0;
            retVal = capuFile.read(vectorPtr, fileSize - totalBytesRead, bytesRead);

            if (retVal == ramses_capu::CAPU_EOF)
            {
                // read to end
                break;
            }
            if (retVal != ramses_capu::CAPU_OK)
            {
                // an error occurred
                break;
            }
            if (bytesRead == 0)
            {
                // read 0 bytes and no EOF
                break;
            }

            totalBytesRead += bytesRead;
            vectorPtr += bytesRead; // Bump the pointer in case end-of-file is not reached yet
        }

        capuFile.close();
        return static_cast<EStatus>(retVal);
    }
}

#endif
