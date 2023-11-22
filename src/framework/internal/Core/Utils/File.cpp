//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LogMacros.h"

#include <cerrno>

namespace ramses::internal
{
    File::File(std::string_view path)
        : m_path(path)
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

    bool File::isOpen() const
    {
        return m_isOpen;
    }

    bool File::isEof() const
    {
        if (m_handle == nullptr)
            return false;
        return std::feof(m_handle) != 0;
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
        std::FILE* handle = std::fopen(m_path.string().c_str(), flags);
        if (handle == nullptr)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::open: fopen {} with flags {} failed, errno is {}", m_path.string(), flags, errno);
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
        if (std::fflush(m_handle) != 0)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::flush: fflush failed for {}, errno is {}", m_path.string(), errno);
            return false;
        }
        return true;
    }

    bool File::close()
    {
        if (m_handle == nullptr)
            return false;

        if (std::fclose(m_handle) != 0)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::close: fclose failed for {}, errno is {}", m_path.string(), errno);
            return false;
        }
        m_handle = nullptr;
        m_isOpen = false;
        return true;
    }

    std::string File::getPath() const
    {
        return m_path.string();
    }

    EStatus File::read(void* buffer, size_t length, size_t& numBytes)
    {
        if (buffer == nullptr)
            return EStatus::Error;
        if (m_handle == nullptr)
            return EStatus::Error;

        size_t result = std::fread(buffer, 1, length, m_handle);

        if (result == length)
        {
            numBytes = result;
            return EStatus::Ok;
        }

        if (std::feof(m_handle) != 0)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::read: fread attempted to read {} bytes from {}, got only {} due to eof", length, m_path.string(), result);
            numBytes = result;
            return EStatus::Eof;
        }

        LOG_ERROR(CONTEXT_FRAMEWORK, "File::read: fread attempted to read {} bytes from {}, got only {} due to read error", length, m_path.string(), result);
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

        const size_t result = std::fwrite(buffer, 1, length, m_handle);
        if (result != length)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::write: fwrite attempted to write {} bytes to {} and failed, {} bytes written", length, m_path.string(), result);
            return false;
        }

        return true;
    }

    bool File::seek(int64_t offset, SeekOrigin origin)
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

        // NOLINTNEXTLINE(google-runtime-int): long is the API type
        if (std::fseek(m_handle, static_cast<long>(offset), nativeOrigin) != 0)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::seek: fseek attempted to seek to {} bytes from {} of file {} and failed, errno is {}", offset, origin, m_path.string(), errno);
            return false;
        }

        return true;
    }

    std::string File::getExtension() const
    {
        auto extension{m_path.extension().string()};
        if (!extension.empty())
            extension.erase(extension.begin()); // remove . from extension
        return extension;
    }

    std::string File::getFileName() const
    {
        return m_path.filename().string();
    }

    bool File::createFile()
    {
        std::FILE* handle = std::fopen(m_path.string().c_str(), "w");
        if (handle == nullptr)
            return false;

        return std::fclose(handle) == 0;
    }

    bool File::isDirectory() const
    {
        std::error_code ec;
        return std::filesystem::is_directory(m_path, ec);
    }

    bool File::exists() const
    {
        std::error_code ec;
        return std::filesystem::exists(m_path, ec);
    }

    bool File::createDirectory()
    {
        std::error_code ec;
        return std::filesystem::create_directory(m_path, ec);
    }

    bool File::remove()
    {
        std::error_code ec;
        return std::filesystem::remove(m_path, ec);
    }

    bool File::getSizeInBytes(size_t& size) const
    {
        std::error_code ec;
        const auto fileSize = std::filesystem::file_size(m_path, ec);
        if (ec)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "File::getSizeInBytes: can't get file size for {}, error code is {}", m_path.string(), ec.message());
            return false;
        }
        size = static_cast<size_t>(fileSize);
        return true;
    }

    bool File::getPos(size_t& position) const
    {
        if (m_handle == nullptr)
            return false;

        if (const auto pos = std::ftell(m_handle); pos >= 0)
        {
            position = static_cast<size_t>(pos);
            return true;
        }

        return false;
    }
}

