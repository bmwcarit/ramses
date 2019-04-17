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

#ifndef RAMSES_CAPU_LINUX_FILE_H
#define RAMSES_CAPU_LINUX_FILE_H

#include <ramses-capu/os/Posix/File.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

namespace ramses_capu
{
    namespace os
    {
        class File: private ramses_capu::posix::File
        {
        public:
            File(const std::string& path);
            File(const File& parent, const std::string& path);
            using ramses_capu::posix::File::open;
            using ramses_capu::posix::File::isOpen;
            using ramses_capu::posix::File::isEof;
            using ramses_capu::posix::File::read;
            using ramses_capu::posix::File::write;
            using ramses_capu::posix::File::seek;
            using ramses_capu::posix::File::getCurrentPosition;
            using ramses_capu::posix::File::flush;
            using ramses_capu::posix::File::close;
            using ramses_capu::posix::File::renameTo;
            using ramses_capu::posix::File::createFile;
            using ramses_capu::posix::File::createDirectory;
            using ramses_capu::posix::File::remove;
            using ramses_capu::posix::File::exists;
            using ramses_capu::posix::File::getFileName;
            using ramses_capu::posix::File::getExtension;
            using ramses_capu::posix::File::getPath;
            using ramses_capu::posix::File::getParentPath;
            using ramses_capu::posix::File::getSizeInBytes;
            using ramses_capu::posix::File::isDirectory;
            status_t copyTo(const std::string& otherFilename);
        };

        inline
        File::File(const std::string& path)
            : ramses_capu::posix::File(path)
        {
        }

        inline
        File::File(const File& parent, const std::string& path)
            : ramses_capu::posix::File(parent, path)
        {
        }

        inline
        status_t File::copyTo(const std::string& otherFilename)
        {
            int_t sourceFile = ::open(mPath.c_str(), O_RDONLY, 0);
            if (-1 == sourceFile)
            {
                return CAPU_ERROR;
            }

            struct stat sourceAttributes;
            int_t status = fstat(sourceFile, &sourceAttributes);
            if (status != 0)
            {
                ::close(sourceFile);
                return CAPU_ERROR;
            }

            // make sure file is not present
            File destFile(otherFilename);
            destFile.remove();

            int_t destinationFile = ::open(otherFilename.c_str(), O_WRONLY | O_CREAT, 0644);
            if (-1 == destinationFile)
            {
                ::close(sourceFile);
                return CAPU_ERROR;
            }
            int bytesSent = sendfile(destinationFile, sourceFile, 0, sourceAttributes.st_size);

            ::close(sourceFile);
            ::close(destinationFile);

            if (bytesSent != sourceAttributes.st_size)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }
    }
}
#endif // RAMSES_CAPU_LINUX_FILE_H
