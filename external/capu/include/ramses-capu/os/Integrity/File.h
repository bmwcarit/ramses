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

#ifndef RAMSES_CAPU_INTEGRITY_FILE_H
#define RAMSES_CAPU_INTEGRITY_FILE_H

#include <limits>
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX __ABS_PATH_MAX // file system max path length
#include <ramses-capu/os/Posix/File.h>

namespace ramses_capu
{
    namespace os
    {
        class File: private ramses_capu::posix::File
        {
        public:
            File(const std::string& path);
            File(const File& parent, const std::string& path);

            status_t open(const FileMode& mode);
            status_t close();

            using ramses_capu::posix::File::isOpen;
            using ramses_capu::posix::File::isEof;
            using ramses_capu::posix::File::read;
            using ramses_capu::posix::File::write;
            using ramses_capu::posix::File::seek;
            using ramses_capu::posix::File::getCurrentPosition;
            using ramses_capu::posix::File::flush;
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
            using ramses_capu::posix::File::copyTo;

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
        status_t File::open(const FileMode& mode)
        {
            return ramses_capu::posix::File::open(mode);
        }

        inline
        status_t File::close()
        {
            return ramses_capu::posix::File::close();
        }

    }
}
#endif // RAMSES_CAPU_INTEGRITY_ARM_V7L_FILE_H
