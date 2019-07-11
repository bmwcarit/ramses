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

#ifndef RAMSES_CAPU_FILE_H
#define RAMSES_CAPU_FILE_H

#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/os/PlatformInclude.h"
#include "ramses-capu/os/FileMode.h"
#include "ramses-capu/os/FileSeekOrigin.h"

#include RAMSES_CAPU_PLATFORM_INCLUDE(File)

namespace ramses_capu
{
    /**
     * Abstract file representation.
     */
    class File: private ramses_capu::os::File
    {
    public:

        /**
         * Create a new instance for a file.
         * @param filepath Path of the file
         */
        File(const std::string& filepath);

        /**
         * try to open the file
         * @param mode to specify the file mode
              READ_ONLY  opens file for reading.
              WRITE_NEW  opens file as an empty file for writing.
              READ_WRITE_EXISTING opens file for reading and writing. The file must exist.
              READ_WRITE_OVERWRITE_OLD opens file for reading and writing. Create a new file also of old one exists.
         */
        status_t open(const FileMode& mode);
        /**
         * create the file
         * @return CAPU_OK if file was created successfully
         */
        status_t createFile();

        /**
         * Create the directory.
         * @return CAPU_OK if directory was created successfully
         */
        status_t createDirectory();

        /**
         * Find out if the file described exists
         * @return true, if file exists otherwise false
         */
        bool exists() const;
        /**
         * Delete the file.
         * @return CAPU_OK, if deletion was successfull.
         */
        status_t remove();

        /**
         * return true if file is open else false
         */
        bool isOpen();

        /**
         * return true if file end was reached
         */
        bool isEof();
        /**
         * Find out if object is a directory.
         * @return True, if File object is a directory.
         */
        bool isDirectory() const;

        /**
         * Get the size of the file.
         * @return The size of the file in Bytes
         */
        status_t getSizeInBytes(uint_t&) const;

        /**
         * Reads data from the stream and store it into the buffer.
         * @param buffer elements to be read
         * @param length of the buffer
         * @param numBytes of bytes read from the stream
         * @return CAPU_EINVAL if params are wrong
                  CAPU_EOF    if end of stream
                  CAPU_ERROR  if invalid state or file not open
         */
        status_t read(char* buffer, uint_t length, uint_t& numBytes);

        /**
         * Writes the given byte buffer to the stream.
         * @param buffer elements to be written
         * @param length of the buffer
         * @return CAPU_OK buffer could be written to the stream
         *        CAPU_ERROR otherwise
         */
        status_t write(const char* buffer, uint_t length);

        /**
         * Moves the position within the file used for reading and writing.
         * @param offset number of bytes to move the position
         * @param origin Origin where to seek from
         * @return CAPU_OK seek was successful
         *        CAPU_ERROR otherwise
         */
        status_t seek(int_t offset, FileSeekOrigin origin);

        /**
         * Get current position within the file.
         * @param position Variable to write current number of bytes from beginning of file to
         * @return CAPU_OK if call was successful
         *        CAPU_ERROR otherwise
         */
        status_t getCurrentPosition(uint_t& position) const;

        /**
         * Writes any unwritten data to the file.
         */
        status_t flush();

        /**
         * Close the stream.
         *@return
         */
        status_t close();

        /**
         * Gets the plain name of the file (the part after the last \\ or /)
         * @return The plain name of the file.
         */
        std::string getFileName() const;

        /**
         * Get the current path or filename of the file.
         * @return The current path or filename of the file.
         */
        const std::string& getPath() const;

        /**
         * Get file extension.
         * @return The filename extension of the file.
         */
        std::string getExtension() const;

        /**
         * Destruct current instance.
         */
        ~File();
    };

    inline
    File::File(const std::string& filepath)
        : ramses_capu::os::File(filepath)
    {
    }

    inline
    status_t File::open(const FileMode& mode)
    {
        return ramses_capu::os::File::open(mode);
    }

    inline
    File::~File()
    {
    }

    inline
    bool
    File::isOpen()
    {
        return ramses_capu::os::File::isOpen();
    }

    inline bool
    File::isEof()
    {
        return ramses_capu::os::File::isEof();
    }

    inline
    status_t
    File::read(char* buffer, uint_t length, uint_t& numBytes)
    {
        return ramses_capu::os::File::read(buffer, length, numBytes);
    }

    inline
    status_t
    File::write(const char* buffer, uint_t length)
    {
        return ramses_capu::os::File::write(buffer, length);
    }

    inline
    status_t
    File::flush()
    {
        return ramses_capu::os::File::flush();
    }

    inline
    status_t
    File::close()
    {
        return ramses_capu::os::File::close();
    }

    inline
    status_t
    File::createFile()
    {
        return ramses_capu::os::File::createFile();
    }

    inline
    status_t
    File::createDirectory()
    {
        return ramses_capu::os::File::createDirectory();
    }

    inline
    status_t
    File::remove()
    {
        return ramses_capu::os::File::remove();
    }

    inline
    bool
    File::exists() const
    {
        return ramses_capu::os::File::exists();
    }

    inline
    std::string File::getFileName() const
    {
        return ramses_capu::os::File::getFileName();
    }

    inline
    const std::string& File::getPath() const
    {
        return ramses_capu::os::File::getPath();
    }

    inline
    ramses_capu::status_t File::getSizeInBytes(ramses_capu::uint_t& size) const
    {
        return ramses_capu::os::File::getSizeInBytes(size);
    }

    inline
    std::string File::getExtension() const
    {
        return ramses_capu::os::File::getExtension();
    }

    inline
    bool File::isDirectory() const
    {
        return ramses_capu::os::File::isDirectory();
    }

    inline
    status_t File::seek(int_t offset, FileSeekOrigin origin)
    {
        return ramses_capu::os::File::seek(offset, origin);
    }

    inline
    status_t File::getCurrentPosition(uint_t& position) const
    {
        return ramses_capu::os::File::getCurrentPosition(position);
    }

}

#endif // RAMSES_CAPU_FILE_H
