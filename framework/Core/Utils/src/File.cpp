//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/File.h"
#include "Utils/LogMacros.h"
#include <cerrno>

namespace ramses_internal
{
    namespace
    {
        // path may not have trailing backslash on windows but remove it on all platforms for consistency
        String RemoveTrailingBackslash(String path)
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
    }

    File::File(String path)
        : m_isOpen(false)
        , m_path(RemoveTrailingBackslash(std::move(path)))
        , m_handle(nullptr)
    {
    }

    File::~File()
    {
        close();
    }

    File::File(File&& other) noexcept
        : m_isOpen(other.m_isOpen)
        , m_path(std::move(other.m_path))
        , m_handle(other.m_handle)
    {
        other.m_isOpen = false;
        other.m_handle = nullptr;
    }

    File& File::operator=(File&& other) noexcept
    {
        if (&other == this)
            return *this;
        m_isOpen = other.m_isOpen;
        m_path = std::move(other.m_path);
        m_handle = other.m_handle;

        other.m_isOpen = false;
        other.m_handle = nullptr;
        return *this;
    }

    bool File::isOpen()
    {
        return m_isOpen;
    }

    bool File::isEof()
    {
        if (m_handle == nullptr)
            return false;
        return feof(m_handle) != 0;
    }


    bool File::open(const Mode& mode)
    {
        // try to open file
        const char* flags = "";
        switch (mode)
        {
        case Mode::ReadOnly:
            flags = "r";
            break;
        case Mode::WriteNew:
            flags = "w";
            break;
        case Mode::WriteExisting:
            flags = "r+";
            break;
        case Mode::WriteOverWriteOld:
            flags = "w+";
            break;
        case Mode::ReadOnlyBinary:
            flags = "rb";
            break;
        case Mode::WriteNewBinary:
            flags = "wb";
            break;
        case Mode::WriteExistingBinary:
            flags = "r+b";
            break;
        case Mode::WriteOverWriteOldBinary:
            flags = "w+b";
            break;
        default:
            flags = "";
        }
        FILE* handle = fopen(m_path.c_str(), flags);
        if (handle == nullptr)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::open: fopen {} with flags {} failed, errno is {}", m_path, flags, errno);
            return false;
        }

        m_handle = handle;
        m_isOpen = true;
        return true;
    }

    bool File::flush()
    {
        if (m_handle == nullptr)
            return false;
        if (fflush(m_handle) != 0)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::flush: fflush failed for {}, errno is {}", m_path, errno);
            return false;
        }
        return true;
    }

    bool File::close()
    {
        if (m_handle == nullptr)
            return false;

        if (fclose(m_handle) != 0)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::close: fclose failed for {}, errno is {}", m_path, errno);
            return false;
        }
        m_handle = nullptr;
        m_isOpen = false;
        return true;
    }

    String File::getPath() const
    {
        return m_path;
    }

    EStatus File::read(void* buffer, size_t length, size_t& numBytes)
    {
        if (buffer == nullptr)
            return EStatus::Error;
        if (m_handle == nullptr)
            return EStatus::Error;

        size_t result = fread(buffer, 1, length, m_handle);

        if (result == length)
        {
            numBytes = result;
            return EStatus::Ok;
        }

        if (feof(m_handle))
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::read: fread attempted to read {} bytes from {}, got only {} due to eof", length, m_path, result);
            numBytes = result;
            return EStatus::Eof;
        }

        LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::read: fread attempted to read {} bytes from {}, got only {} due to read error", length, m_path, result);
        return EStatus::Error;
    }

    bool File::write(const void* buffer, size_t length)
    {
        if (buffer == nullptr)
            return false;
        if (m_handle == nullptr)
            return false;

        if (length == 0)
            return true;

        size_t result = fwrite(buffer, length, 1, m_handle);
        if (result != 1u)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::write: fwrite attempted to write {} bytes to {} and failed", length, m_path);
            return false;
        }

        return true;
    }

    bool File::seek(std::intptr_t offset, SeekOrigin origin)
    {
        if (m_handle == nullptr)
            return false;

        int nativeOrigin = 0;
        switch (origin)
        {
        case SeekOrigin::BeginningOfFile:
            nativeOrigin = SEEK_SET;
            break;
        case SeekOrigin::RelativeToCurrentPosition:
            nativeOrigin = SEEK_CUR;
            break;
        }
        size_t result = fseek(m_handle, static_cast<long>(offset), nativeOrigin);

        if (0 != result)
        {
            LOG_ERROR_P(CONTEXT_FRAMEWORK, "File::seek: fseek attempted to seek to {} bytes from {} of file {} and failed, errno is {}", offset, origin, m_path, errno);
            return false;
        }

        return true;
    }

    String File::getExtension() const
    {
        const size_t position = m_path.rfind('.');
        if (position == String::npos)
        {
            // index not found
            return String();
        }
        return String(std::string(m_path.stdRef(), position + 1));
    }

    String File::getFileName() const
    {
        size_t lastSeparator = m_path.rfind('/');
        if (lastSeparator == String::npos)
            lastSeparator = m_path.rfind('\\');

        if (lastSeparator != String::npos)
            return String(std::string(m_path.stdRef(), lastSeparator + 1));

        return m_path;
    }

    bool File::createFile()
    {
        FILE* handle = fopen(m_path.c_str(), "w");
        if (handle == nullptr)
            return false;

        fclose(handle);
        return true;
    }
}

#ifdef _WIN32

#include "PlatformAbstraction/MinimalWindowsH.h"

namespace ramses_internal
{
    bool File::createDirectory()
    {
        BOOL status = CreateDirectoryA(m_path.c_str(), 0) == TRUE;
        if (status != TRUE)
            return false;
        return true;
    }

    bool File::remove()
    {
        BOOL status = FALSE;
        if (isDirectory())
        {
            status = RemoveDirectoryA(m_path.c_str());
        }
        else
        {
            status = DeleteFileA(m_path.c_str());
        }
        if (status == TRUE)
            return true;
        return false;
    }

    bool File::isDirectory() const
    {
        DWORD dwAttributes = GetFileAttributesA(m_path.c_str());
        return (dwAttributes != INVALID_FILE_ATTRIBUTES) && ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }

    bool File::exists() const
    {
        DWORD dwAttributes = GetFileAttributesA(m_path.c_str());
        return (dwAttributes != INVALID_FILE_ATTRIBUTES);
    }

    bool File::getPos(size_t& position) const
    {
        __int64 pos = ftell(m_handle);
        if (pos >= 0)
        {
            position = static_cast<size_t>(pos);
            return true;
        }
        return false;
    }

    // TODO(tobias) does not work on write opened file, should be reworked to use m_handle if m_isOpen
    bool File::getSizeInBytes(size_t& size) const
    {
        HANDLE fileHandle = CreateFileA(m_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fileHandle == nullptr)
        {
            return false;
        }
        LARGE_INTEGER tempSize;
        BOOL status = GetFileSizeEx(fileHandle, &tempSize);
        CloseHandle(fileHandle);
        if (status != TRUE)
        {
            return false;
        }
        size = static_cast<size_t>(tempSize.QuadPart);
        return true;
    }
}


#else // posix
#include <sys/stat.h>
#include <unistd.h>

namespace ramses_internal
{
    bool File::getSizeInBytes(size_t& size) const
    {
        struct stat tmp;
        if (stat(m_path.c_str(), &tmp) != 0)
            return false;

        size = tmp.st_size;
        return true;
    }

    bool File::isDirectory() const
    {
        struct stat tmp;
        return stat(m_path.c_str(), &tmp) == 0 && S_ISDIR(tmp.st_mode);  // NOLINT(hicpp-signed-bitwise) ignore bad stuff in macro
    }

    bool File::createDirectory()
    {
        if (mkdir(m_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO ) != 0) // mode 0777
            return false;
        return true;
    }

    bool File::remove()
    {
        int status = -1;
        if (isDirectory())
        {
            status = ::rmdir(m_path.c_str());
        }
        else
        {
            status = ::remove(m_path.c_str());
        }

        if (status == 0)
            return true;

        return false;
    }

    bool File::exists() const
    {
        struct stat fileStats;
        if (stat(m_path.c_str(), &fileStats) != 0)
            return false;

        return true;
    }

    bool File::getPos(size_t& position) const
    {
        const off_t pos = ftello(m_handle);
        if (pos >= 0)
        {
            position = pos;
            return true;
        }

        return false;
    }
}
#endif
