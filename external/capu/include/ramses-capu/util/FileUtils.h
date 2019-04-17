/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_FILEUTILS_H
#define RAMSES_CAPU_FILEUTILS_H

#include "ramses-capu/os/File.h"
#include "ramses-capu/os/Memory.h"
#include <vector>

namespace ramses_capu
{
    /**
    * Helper methods to work with files.
    */
    class FileUtils
    {
    public:
        /**
        * Reads all text from a file.
        * @param file The file containing the text.
        * @return The file content as string.
        */
        static std::string readAllText(File& file);

        /**
        * Reads all bytes from a file.
        * @param file The file containing the bytes.
        * @param result A vector which the resulting bytes will be appended to.
        * @return CAPU_OK if the file was read successfully.
        */
        static status_t readAllBytes(File& file, std::vector<Byte>& result);

        /**
        * Writes all given text in a file. Existing content will get overwritten.
        * @param file The file into which to content will get written.
        * @param content The content that should go into the file.
        * @return The return value.
        */
        static status_t writeAllText(File& file, const std::string& content);

        /**
        * Writes all bytes to a file. Existing content will be overwritten.
        * @param file The file which the content will get written to.
        * @param buffer A pointer to an input buffer with byte data.
        * @param numberOfBytesToWrite The number of bytes to write.
        * @return CAPU_OK if the file was written successfully.
        */
        static status_t writeAllBytes(File& file, const Byte* buffer, uint32_t numberOfBytesToWrite);
    };

    inline status_t FileUtils::readAllBytes(File& file, std::vector<Byte>& result)
    {
        ramses_capu::uint_t fileSize;
        if (file.getSizeInBytes(fileSize) != ramses_capu::CAPU_OK)
        {
            // error determining file size
            return CAPU_ERROR;
        }

        result.resize(fileSize);
        file.open(ramses_capu::READ_ONLY_BINARY);
        ramses_capu::uint_t totalBytesRead = 0;
        status_t retVal = CAPU_ERROR;

        // Read directly into the vector
        char* vectorPtr = reinterpret_cast<char*>(result.data());

        while (totalBytesRead < fileSize)
        {
            ramses_capu::uint_t bytesRead = 0;
            retVal = file.read(vectorPtr, fileSize - totalBytesRead, bytesRead);

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

        file.close();
        return retVal;
    }

    inline status_t FileUtils::writeAllBytes(File& file, const Byte* buffer, uint32_t numberOfBytesToWrite)
    {
        if (file.exists())
        {
            file.remove();
        }
        file.open(ramses_capu::READ_WRITE_OVERWRITE_OLD_BINARY);
        file.write(reinterpret_cast<const char*>(buffer), numberOfBytesToWrite);
        file.flush();
        file.close();
        return CAPU_OK;
    }

    inline std::string FileUtils::readAllText(File& file)
    {
        // read the file
        ramses_capu::uint_t fileSize;
        if (file.getSizeInBytes(fileSize) != ramses_capu::CAPU_OK)
        {
            // error determining file size
            return "";
        }
        ramses_capu::uint_t readBytes = 0;
        char* buffer = new char[fileSize + 1];
        ramses_capu::Memory::Set(buffer, 0, fileSize + 1);
        file.open(ramses_capu::READ_ONLY);
        ramses_capu::uint_t totalBytes = 0;
        status_t retVal = CAPU_ERROR;
        while (totalBytes < fileSize)
        {
            retVal = file.read(&buffer[totalBytes], fileSize - totalBytes, readBytes);
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
            if (readBytes <= 0)
            {
                // read 0 bytes and no EOF (possible?)
                break;
            }
            totalBytes += readBytes;
        }
        file.close();
        if (retVal == CAPU_OK || retVal == CAPU_EOF) // read all content
        {
            std::string result(buffer);
            delete[] buffer;
            return result;
        }
        else
        {
            delete[] buffer;
            return "";
        }
    }

    inline status_t FileUtils::writeAllText(File& file, const std::string& content)
    {
        if (file.exists())
        {
            file.remove();
        }
        file.open(ramses_capu::READ_WRITE_OVERWRITE_OLD);
        file.write(content.c_str(), content.size());
        file.flush();
        file.close();
        return CAPU_OK;
    }
}

#endif /* RAMSES_CAPU_FILEUTILS_H */
