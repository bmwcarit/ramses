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
            File(const String& path);
            File(const File& parent, const ramses_capu::String& path);
            status_t open(const FileMode& mode);
            bool isOpen();
            bool isEof();
            status_t read(char* buffer, uint_t length, uint_t& numBytes);
            status_t write(const char* buffer, uint_t length);
            using generic::File::seek;
            status_t getCurrentPosition(uint_t& position) const;
            status_t flush();
            status_t close();
            status_t renameTo(const ramses_capu::String& newPath);
            status_t copyTo(const ramses_capu::String& otherPath);
            status_t createFile();
            status_t createDirectory();
            status_t remove();
            using generic::File::getFileName;
            using generic::File::getExtension;
            using generic::File::getPath;
            String getParentPath(bool& success) const;
            bool isDirectory() const;
            bool exists() const;
            status_t getSizeInBytes(uint_t&) const;
            ~File();

        protected:
            using generic::File::mPath;
            using generic::File::mHandle;
        private:
            bool  mIsOpen;
            static String removeTrailingBackslash(String path);
        };

        inline
        File::File(const String& path)
            : generic::File(removeTrailingBackslash(path))
            , mIsOpen(false)
        {
        }

        inline
        File::File(const File& parent, const ramses_capu::String& path)
            : generic::File(parent, removeTrailingBackslash(path))
            , mIsOpen(false)
        {
        }

        inline
        String File::removeTrailingBackslash(String path)
        {
            if (path.endsWith("\\") || path.endsWith("/"))
            {
                if (!path.endsWith(":\\") && !path.endsWith(":/"))
                {
                    path.truncate(path.getLength() - 1);
                }
            }
            return path;
        }

        inline
        String File::getParentPath(bool& success) const
        {
            char buffer[1000]; // TODO
            char* filePointer = 0;
            int_t length = GetFullPathNameA(getPath().c_str(), sizeof(buffer), buffer, &filePointer);
            if (filePointer)
            {
                // "trim" last path component
                *(filePointer) = 0;
            }
            else
            {
                // no path found
                length = 0;
                buffer[0] = 0;
            }
            success = length > 0;
            return String(buffer);
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
        status_t File::renameTo(const ramses_capu::String& newPath)
        {
            int_t status = MoveFileExA(mPath.c_str(), newPath.c_str(), 0);
            if (status == 0)
            {
                return CAPU_ERROR;
            }
            mPath = newPath;
            return CAPU_OK;
        }

        inline
        status_t File::copyTo(const ramses_capu::String& otherPath)
        {
            int_t status = CopyFileA(mPath.c_str(), otherPath.c_str(), false);
            if (status == 0)
            {
                return CAPU_ERROR;
            }
            return CAPU_OK;
        }

        inline
        status_t File::open(const FileMode& mode)
        {
            errno_t error;
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
            error  = fopen_s(&mHandle, mPath.c_str(), flags);
            if (error == 0)
            {
                mIsOpen = true;
                return CAPU_OK;
            }
            return CAPU_ERROR;
        }

        inline
        bool
        File::isOpen()
        {
            return mIsOpen;
        }

        inline
        bool File::isEof()
        {
            if (mHandle == NULL)
            {
                return false;
            }
            return (feof(mHandle) != 0);
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

            size_t result = fwrite(buffer, 1, length, mHandle);
            if (result != length)
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
        File::~File()
        {
            if (mHandle != NULL)
            {
                fclose(mHandle);
            }
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
