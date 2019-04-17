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

#ifndef RAMSES_CAPU_GENERIC_FILE_H
#define RAMSES_CAPU_GENERIC_FILE_H

#include "ramses-capu/Config.h"
#include "ramses-capu/os/StringUtils.h"
#include <string>

namespace ramses_capu
{
    namespace generic
    {
        class File
        {
        public:
            File(const std::string& path);
            File(const File& parent, const std::string& path);
            const std::string getFileName() const;
            const std::string getExtension() const;
            const std::string& getPath() const;
            status_t seek(int_t offset, FileSeekOrigin origin);
            ~File();

        protected:
            std::string mPath;
            FILE* mHandle;
        };

        inline
        File::File(const std::string& path)
            : mPath(path)
            , mHandle(NULL)
        {
        }

        inline
        File::File(const File& parent, const std::string& path)
            : mPath(parent.getPath())
            , mHandle(NULL)
        {
            mPath.append("/").append(path);
        }

        inline
        File::~File()
        {
        }

        inline
        const std::string& File::getPath() const
        {
            return mPath;
        }

        inline
        const std::string File::getExtension() const
        {
            int_t position = StringUtils::LastIndexOf(mPath.c_str(), '.');
            if (position < 0)
            {
                // index not found
                return std::string();
            }
            return std::string(mPath, position + 1);
        }

        inline
        const std::string File::getFileName() const
        {
            int_t lastSeparator = StringUtils::LastIndexOf(mPath.c_str(), '/');
            if (lastSeparator == -1)
            {
                lastSeparator = StringUtils::LastIndexOf(mPath.c_str(), '\\');
            }
            if (lastSeparator != -1)
            {
                return std::string(mPath, lastSeparator + 1);
            }
            else
            {
                return mPath;
            }
        }

        inline
        status_t File::seek(int_t offset, FileSeekOrigin origin)
        {
            if (mHandle == NULL)
            {
                return CAPU_ERROR;
            }

            int nativeOrigin = 0;
            switch (origin)
            {
            case FROM_BEGINNING: nativeOrigin = SEEK_SET;
                break;
            case FROM_CURRENT_POSITION: nativeOrigin = SEEK_CUR;
                break;
            }
            size_t result = fseek(mHandle, static_cast<long>(offset), nativeOrigin);

            if (0 == result)
            {
                return CAPU_OK;
            }

            return CAPU_ERROR;
        }
    }
}

#endif //RAMSES_CAPU_GENERIC_FILE_H
