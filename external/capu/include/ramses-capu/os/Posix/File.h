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

#ifndef RAMSES_CAPU_UNIXBASED_FILE_H
#define RAMSES_CAPU_UNIXBASED_FILE_H

#include "ramses-capu/os/Generic/File.h"
#include "ramses-capu/os/FileMode.h"
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <libgen.h>

namespace ramses_capu
{
    namespace posix
    {
        class File : private generic::File
        {
        public:
            using generic::File::File;
            using generic::File::open;
            using generic::File::isOpen;
            using generic::File::isEof;
            using generic::File::seek;
            using generic::File::read;
            using generic::File::write;
            using generic::File::flush;
            using generic::File::close;
            status_t getCurrentPosition(uint_t& position) const;
            status_t createFile();
            status_t createDirectory();
            status_t remove();
            using generic::File::getFileName;
            using generic::File::getExtension;
            using generic::File::getPath;
            status_t getSizeInBytes(uint_t& size) const;
            bool isDirectory() const;
            bool exists() const;
        protected:
            using generic::File::mIsOpen;
            using generic::File::mPath;
            using generic::File::mHandle;
        };

        inline
        status_t File::getSizeInBytes(uint_t& size) const
        {
            struct stat tmp;
            int_t status = stat(mPath.c_str(), &tmp);
            if (status != 0)
            {
                return CAPU_ERROR;
            }
            size = tmp.st_size;
            return CAPU_OK;
        }

        inline
        bool File::isDirectory() const
        {
            struct stat tmp;
            return stat(mPath.c_str(), &tmp) == 0 && S_ISDIR(tmp.st_mode);
        }

        inline
        status_t File::createFile()
        {
            FILE* handle = fopen(mPath.c_str(), "w");
            if (handle == NULL)
            {
                return CAPU_ERROR;
            }

            fclose(handle);
            return CAPU_OK;
        }

        inline
        status_t File::createDirectory()
        {
            int_t status = mkdir(mPath.c_str(), 0777);
            if (status != 0)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
        status_t File::remove()
        {
            int_t status = -1;
            if (isDirectory())
            {
                status = rmdir(mPath.c_str());
            }
            else
            {
                status = ::remove(mPath.c_str());
            }

            if (status == 0)
            {
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        bool File::exists() const
        {
            struct stat fileStats;
            int_t status = stat(mPath.c_str(), &fileStats);
            if (status != 0)
            {
                return false;
            }
            return true;
        }

        inline
        status_t File::getCurrentPosition(uint_t& position) const
        {
            const off_t pos = ftello(mHandle);
            if (pos >= 0)
            {
                position = pos;
                return CAPU_OK;
    }

            return CAPU_ERROR;
        }
    }
}

#endif // RAMSES_CAPU_UNIXBASED_FILE_H
