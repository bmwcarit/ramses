//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_STRINGOUTPUTSTREAM_H
#define RAMSES_RAMSES_STRINGOUTPUTSTREAM_H

#include "Collections/String.h"
#include "Utils/StringUtils.h"
#include <inttypes.h>

namespace ramses_internal
{
    class StringOutputStream final
    {
    public:
        explicit StringOutputStream(UInt initialCapacity = 16);
        explicit StringOutputStream(String initialContent);
        explicit StringOutputStream(std::string initialContent);

        StringOutputStream& operator<<(const Char* value);

        template <typename T, typename = decltype(::fmt::to_string(std::declval<T>()))>
        StringOutputStream& operator<<(const T& value)
        {
            m_buffer.reserve(m_buffer.size() + fmt::formatted_size("{}", value));
            fmt::format_to(std::back_inserter(m_buffer), "{}", value);
            return *this;
        }

        void clear();

        void reserve(UInt capacity);
        UInt capacity() const;

        const Char* c_str() const;
        UInt32 size() const;

        String release();
        const std::string& data() const;

        template <typename T>
        static String ToString(const T& value);

    private:
        std::string m_buffer;
    };

    inline
    StringOutputStream::StringOutputStream(UInt initialCapacity)
    {
        m_buffer.reserve(initialCapacity);
    }

    inline
    StringOutputStream::StringOutputStream(String initialContent)
        : m_buffer(std::move(initialContent.stdRef()))
    {
    }

    inline
    StringOutputStream::StringOutputStream(std::string initialContent)
        : m_buffer(std::move(initialContent))
    {
    }

    inline
    UInt32 StringOutputStream::size() const
    {
        return static_cast<uint32_t>(m_buffer.size());
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Char* value)
    {
        if (value)
            fmt::format_to(std::back_inserter(m_buffer), "{}", value);
        return *this;
    }

    inline const Char* StringOutputStream::c_str() const
    {
        return m_buffer.c_str();
    }

    inline
    void StringOutputStream::clear()
    {
        m_buffer.clear();
    }

    inline
    void StringOutputStream::reserve(UInt capacity)
    {
        m_buffer.reserve(capacity);
    }

    inline
    UInt StringOutputStream::capacity() const
    {
        return m_buffer.capacity();
    }

    inline
    const std::string& StringOutputStream::data() const
    {
        return m_buffer;
    }

    inline
    String StringOutputStream::release()
    {
        std::string tmp;
        tmp.swap(m_buffer);
        return String(std::move(tmp));
    }

    template <typename T>
    inline String StringOutputStream::ToString(const T& value)
    {
        StringOutputStream sos;
        sos << value;
        return sos.release();
    }
}

#endif
