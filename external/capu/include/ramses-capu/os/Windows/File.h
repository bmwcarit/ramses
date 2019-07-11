/*
 * Copyright (C) 2018 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_WINDOWS_FILE_H
#define RAMSES_CAPU_WINDOWS_FILE_H

#include "ramses-capu/os/Generic/File.h"
#include "ramses-capu/os/FileMode.h"
#include "ramses-capu/os/Windows/MinimalWindowsH.h"


namespace ramses_capu
{
    namespace os
    {
        class File : private generic::File
        {
        public:
            File(const std::string& path);
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
            bool isDirectory() const;
            bool exists() const;
            status_t getSizeInBytes(uint_t&) const;

        protected:
            using generic::File::mIsOpen;
            using generic::File::mPath;
            using generic::File::mHandle;
        private:
            static std::string removeTrailingBackslash(std::string path);
        };

        inline
        File::File(const std::string& path)
            : generic::File(removeTrailingBackslash(path))
        {
        }

        inline
        std::string File::removeTrailingBackslash(std::string path)
        {
            const auto len = path.size();
            if (len > 0 && (path[len-1] == '\\' || path[len-1] == '/'))
            {
                if (len < 2 || path[len-2] != ':')
                {
                    path.resize(len - 1);
                }
            }
            return path;
        }

        inline
        status_t File::createFile()
        {
            HANDLE handle = CreateFileA(mPath.c_str(), 0, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            bool status = (handle != INVALID_HANDLE_VALUE);
            CloseHandle(handle);
            if (status)
            {
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        status_t File::createDirectory()
        {
            BOOL status = CreateDirectoryA(mPath.c_str(), 0) == TRUE;
            if (status != TRUE)
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
                status = RemoveDirectoryA(mPath.c_str());
            }
            else
            {
                status = DeleteFileA(mPath.c_str());
            }
            if (status != 0)
            {
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        bool File::isDirectory() const
        {
            DWORD dwAttributes = GetFileAttributesA(mPath.c_str());
            return (dwAttributes != INVALID_FILE_ATTRIBUTES) && ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        }

        inline
        bool File::exists() const
        {
            DWORD dwAttributes = GetFileAttributesA(mPath.c_str());
            return (dwAttributes != INVALID_FILE_ATTRIBUTES);
        }

        inline
        status_t File::getCurrentPosition(uint_t& position) const
        {
            __int64 pos = ftell(mHandle);
            if (pos >= 0)
            {
                position = static_cast<uint_t>(pos);
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        status_t File::getSizeInBytes(uint_t& size) const
        {
            HANDLE fileHandle = CreateFileA(mPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fileHandle == NULL)
            {
                return CAPU_ENOT_EXIST;
            }
            LARGE_INTEGER tempSize;
            BOOL status = GetFileSizeEx(fileHandle, &tempSize);
            CloseHandle(fileHandle);
            if (status != TRUE)
            {
                return CAPU_ERROR;
            }
            size = static_cast<uint_t>(tempSize.QuadPart);
            return CAPU_OK;
        }
    }
}

#endif
