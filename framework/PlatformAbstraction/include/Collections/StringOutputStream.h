//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_STRINGOUTPUTSTREAM_H
#define RAMSES_RAMSES_STRINGOUTPUTSTREAM_H

#include "PlatformAbstraction/Macros.h"
#include "fmt/format.h"
#include <string>
#include <cinttypes>

namespace ramses_internal
{
    class StringOutputStream final
    {
    public:
        explicit StringOutputStream(size_t initialCapacity = 160);
        explicit StringOutputStream(std::string initialContent);

        StringOutputStream& operator<<(const char* value);

        template <typename T, typename = decltype(::fmt::to_string(std::declval<T>()))>
        StringOutputStream& operator<<(const T& value)
        {
            m_buffer.reserve(m_buffer.size() + fmt::formatted_size("{}", value));
            fmt::format_to(std::back_inserter(m_buffer), "{}", value);
            return *this;
        }

        void reserve(size_t capacity);
        RNODISCARD size_t capacity() const;

        RNODISCARD const char* c_str() const;
        RNODISCARD size_t size() const;

        RNODISCARD std::string release();
        RNODISCARD const std::string& data() const;

    private:
        std::string m_buffer;
    };

    inline
    StringOutputStream::StringOutputStream(size_t initialCapacity)
    {
        m_buffer.reserve(initialCapacity);
    }

    inline
    StringOutputStream::StringOutputStream(std::string initialContent)
        : m_buffer(std::move(initialContent))
    {
    }

    inline
    size_t StringOutputStream::size() const
    {
        return static_cast<uint32_t>(m_buffer.size());
    }

    inline StringOutputStream& StringOutputStream::operator<<(const char* value)
    {
        if (value)
            fmt::format_to(std::back_inserter(m_buffer), "{}", value);
        return *this;
    }

    inline const char* StringOutputStream::c_str() const
    {
        return m_buffer.c_str();
    }

    inline
    void StringOutputStream::reserve(size_t capacity)
    {
        m_buffer.reserve(capacity);
    }

    inline
    size_t StringOutputStream::capacity() const
    {
        return m_buffer.capacity();
    }

    inline
    const std::string& StringOutputStream::data() const
    {
        return m_buffer;
    }

    inline
    std::string StringOutputStream::release()
    {
        std::string tmp;
        tmp.swap(m_buffer);
        return tmp;
    }
}

#endif
