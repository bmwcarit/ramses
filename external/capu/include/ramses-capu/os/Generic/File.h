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
            status_t open(const FileMode& mode);
            bool isOpen();
            bool isEof();
            status_t read(char* buffer, uint_t length, uint_t& numBytes);
            status_t write(const char* buffer, uint_t length);
            status_t flush();
            status_t close();
            std::string getFileName() const;
            std::string getExtension() const;
            const std::string& getPath() const;
            status_t seek(int_t offset, FileSeekOrigin origin);
            ~File();

        protected:
            bool mIsOpen;
            std::string mPath;
            FILE* mHandle;
        };

        inline
        File::File(const std::string& path)
            : mIsOpen(false)
            , mPath(path)
            , mHandle(NULL)
        {
        }

        inline
        File::~File()
        {
            if (mHandle != NULL)
            {
                fclose(mHandle);
            }
        }

        inline
        bool
        File::isOpen()
        {
            return mIsOpen;
        }

        inline
        bool
        File::isEof()
        {
            if (mHandle == NULL)
            {
                return false;
            }
            return (feof(mHandle) != 0);
        }


        inline
        status_t File::open(const FileMode& mode)
        {
            // try to open file
            const char* flags = "";
            switch (mode)
            {
            case READ_ONLY:
                flags = "r";
                break;
            case WRITE_NEW:
                flags = "w";
                break;
            case READ_WRITE_EXISTING:
                flags = "r+";
                break;
            case READ_WRITE_OVERWRITE_OLD:
                flags = "w+";
                break;
            case READ_ONLY_BINARY:
                flags = "rb";
                break;
            case WRITE_NEW_BINARY:
                flags = "wb";
                break;
            case READ_WRITE_EXISTING_BINARY:
                flags = "r+b";
                break;
            case READ_WRITE_OVERWRITE_OLD_BINARY:
                flags = "w+b";
                break;
            default:
                flags = "";
            }
            mHandle  = fopen(mPath.c_str(), flags);
            if (mHandle != NULL)
            {
                mIsOpen = true;
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        status_t
        File::read(char* buffer, uint_t length, uint_t& numBytes)
        {
            if (buffer == NULL)
            {
                return CAPU_EINVAL;
            }
            if (mHandle == NULL)
            {
                return CAPU_ERROR;
            }

            size_t result = fread(buffer, 1, length, mHandle);

            if (result == length)
            {
                numBytes = result;
                return CAPU_OK;
            }

            if (feof(mHandle))
            {
                numBytes = result;
                return CAPU_EOF;
            }
            return CAPU_ERROR;
        }

        inline
        status_t
        File::write(const char* buffer, uint_t length)
        {
            if (buffer == NULL)
            {
                return CAPU_EINVAL;
            }
            if (mHandle == NULL)
            {
                return CAPU_ERROR;
            }
            if (length == 0)
            {
                return CAPU_OK;
            }

            size_t result = fwrite(buffer, length, 1, mHandle);
            if (result != 1u)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
        status_t
        File::flush()
        {
            if (mHandle != NULL)
            {
                int_t error = fflush(mHandle);
                if (error == 0)
                {
                    return CAPU_OK;
                }
            }
            return CAPU_ERROR;
        }

        inline
        status_t
        File::close()
        {
            if (mHandle != NULL)
            {
                fclose(mHandle);
                mHandle = NULL;
                mIsOpen = false;
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        const std::string& File::getPath() const
        {
            return mPath;
        }

        inline
        std::string File::getExtension() const
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
        std::string File::getFileName() const
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
