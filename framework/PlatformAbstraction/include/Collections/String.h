//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_STRING_H
#define RAMSES_PLATFORM_STRING_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/Macros.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/container/Hash.h"
#include "fmt/format.h"
#include <string>
#include <cctype>

namespace ramses_internal
{
    class String
    {
    public:
        String() = default;
        String(const Char* data);
        String(UInt initialSize, Char character);
        String(const Char* data, UInt start, UInt end);
        explicit String(const std::string& other);
        explicit String(std::string&& other);
        String(const String& other) = default;
        String(String&& other) RNOEXCEPT = default;
        ~String() = default;
        const Char* c_str() const;
        Char at(UInt position) const;
        Int find(const String& substring, UInt startPos = 0) const;
        Int find(char ch, UInt offset = 0) const;

        String& operator=(const String& other) = default;
        String& operator=(String&& other) RNOEXCEPT = default;
        String& operator=(const std::string& other);
        String& operator=(std::string&& other);
        String& operator=(Char character);
        String& operator=(const Char* other);

        String operator+(const String& rOperand) const;
        String operator+(const std::string& rOperand) const;
        String operator+(const Char* rOperand) const;
        void operator+=(Char character);
        void operator+=(const Char* other);
        void operator+=(const String& other);

        Char& operator[](UInt index);
        Char operator[](UInt index) const;

        bool operator==(const String& other) const;
        bool operator==(const std::string& other) const;
        bool operator==(const char* other) const;
        bool operator!=(const String& other) const;
        bool operator!=(const std::string& other) const;
        bool operator!=(const char* other) const;

        String& append(const String& other);
        String& append(const std::string& other);
        String& append(const Char* other);

        String substr(UInt start, Int length) const;
        UInt size() const;
        void toUpperCase();
        void toLowerCase();
        void clear();
        bool empty() const;
        void resize(UInt newSize);
        char* data();
        const char* data() const;
        void reserve(UInt capacity);
        UInt capacity() const;
        bool startsWith(const String& other) const;
        bool endsWith(const String& other) const;
        bool operator<(const String& other) const;
        bool operator>(const String& other) const;
        Int rfind(char ch) const;

        /**
         * Swaps this string with another
         * @param other The other string
         * @return Reference to this string
         */
        String& swap(String& other);

        const std::string& stdRef() const;
        std::string& stdRef();

    private:
        std::string m_string;
    };

    inline IOutputStream& operator<<(IOutputStream& stream, const std::string& value)
    {
        const uint32_t len = static_cast<uint32_t>(value.size());
        stream << len;
        return stream.write(value.c_str(), len);
    }

    inline IOutputStream& operator<<(IOutputStream& stream, const String& value)
    {
        return stream << value.stdRef();
    }

    inline IInputStream& operator>>(IInputStream& stream, String& value)
    {
        uint32_t length = 0;
        stream >> length; // first read the length of the string
        String retValue;
        if (length > 0)
        {
            retValue.resize(length);
            stream.read(retValue.data(), length);
        }
        value.swap(retValue);
        return stream;
    }

    inline IInputStream& operator>>(IInputStream& stream, std::string& value)
    {
        String str;
        stream >> str;
        value.swap(str.stdRef());
        return stream;
    }

    static_assert(std::is_nothrow_move_constructible<String>::value, "String must be movable");
    static_assert(std::is_nothrow_move_assignable<String>::value, "String must be movable");

    // free comparison functions
    inline bool operator==(const char* a, const String& b)
    {
        return b == a;
    }

    inline bool operator!=(const char* a, const String& b)
    {
        return b != a;
    }

    inline bool operator==(const std::string& a, const String& b)
    {
        return b == a;
    }

    inline bool operator!=(const std::string& a, const String& b)
    {
        return b != a;
    }

    /*
     * Implementation String
     */

    inline String::String(const Char* other)
    {
        if (other && *other)
            m_string = other;
    }

    inline String::String(const Char* data, UInt start, UInt end)
    {
        // no data
        if (!data)
        {
            return;
        }

        // end before start
        if (end < start)
        {
            return;
        }

        // start too big
        const UInt size = strnlen(data, end + 1);
        if (start > size)
        {
            return;
        }

        // end too big, adjust to point to the last character
        UInt theend = end;
        if (theend >= size)
        {
            theend = size - 1;
        }

        // do the work
        const char* startdata = data + start;
        const size_t endPos = theend - start + 1;
        m_string.assign(startdata, startdata + endPos);
    }

    inline String::String(UInt initialSize, Char character)
        : m_string(initialSize, character)
    {
    }

    inline String::String(const std::string& other)
        : m_string(other)
    {
    }

    inline String::String(std::string&& other)
        : m_string(std::move(other))
    {
    }

    inline void String::resize(UInt newSize)
    {
        m_string.resize(newSize);
    }

    inline const char* String::data() const
    {
        return m_string.data();
    }

    inline char* String::data()
    {
        return m_string.data() ? &m_string[0] : nullptr;
    }

    inline String String::operator+(const String& rOperand) const
    {
        String result(*this);
        return result.append(rOperand);
    }

    inline String String::operator+(const std::string& rOperand) const
    {
        String result(*this);
        return result.append(rOperand);
    }

    inline String String::operator+(const Char* rOperand) const
    {
        String result(c_str());
        return result.append(rOperand);
    }

    inline String operator+(const Char* lOperand, const String& rOperand)
    {
        String result(lOperand);
        return result.append(rOperand.c_str());
    }

    inline String& String::operator=(const Char* other)
    {
        if (other)
            m_string = other;
        else
            m_string.clear();
        return *this;
    }

    inline bool String::operator==(const String& other) const
    {
        return m_string == other.m_string;
    }

    inline bool String::operator==(const std::string& other) const
    {
        return m_string == other;
    }

    inline bool String::operator==(const char* other) const
    {
        return m_string == other;
    }

    inline bool String::operator!=(const String& other) const
    {
        return !operator==(other);
    }

    inline bool String::operator!=(const std::string& other) const
    {
        return m_string != other;
    }

    inline bool String::operator!=(const char* other) const
    {
        return !operator==(other);
    }

    inline void String::operator+=(Char character)
    {
        char tmp[2] = {character, '\0'};
        operator+=(tmp);
    }

    inline void String::operator+=(const Char* other)
    {
        append(other);
    }

    inline void String::operator+=(const String& other)
    {
        m_string += other.m_string;
    }

    inline Char& String::operator[](UInt index)
    {
        return m_string[index];
    }

    inline Char String::operator[](UInt index) const
    {
        return m_string[index];
    }

    inline String& String::operator=(const std::string& other)
    {
        m_string = other;
        return *this;
    }

    inline String& String::operator=(std::string&& other)
    {
        m_string = std::move(other);
        return *this;
    }

    inline String& String::operator=(Char character)
    {
        char tmp[2] = {character, '\0'};
        return operator=(tmp);
    }

    inline String& String::append(const String& other)
    {
        m_string += other.m_string;
        return *this;
    }

    inline String& String::append(const std::string& other)
    {
        m_string += other;
        return *this;
    }

    inline void String::toUpperCase()
    {
        for (auto& c : m_string)
            c = static_cast<char>(std::toupper(c));
    }

    inline void String::toLowerCase()
    {
        for (auto& c : m_string)
            c = static_cast<char>(std::tolower(c));
    }

    inline String& String::append(const Char* other)
    {
        if (other)
            m_string += other;
        return *this;
    }

    inline String String::substr(UInt start, Int length) const
    {
        if (length < 0)
            length = m_string.size();
        return String(c_str(), start, start + length - 1);
    }

    inline const Char* String::c_str() const
    {
        return m_string.c_str();
    }

    inline UInt String::size() const
    {
        return m_string.size();
    }

    inline String& String::swap(String& other)
    {
        m_string.swap(other.m_string);
        return *this;
    }

    inline Int String::find(const String& substring, UInt startPos) const
    {
        size_t res = m_string.find(substring.m_string, startPos);
        if (res == std::string::npos)
            return -1;
        return res;
    }

    inline Int String::find(char ch, UInt startPos) const
    {
        size_t res = m_string.find(ch, startPos);
        if (res == std::string::npos)
            return -1;
        return res;
    }

    inline Char String::at(UInt position) const
    {
        if (position < size())
            return m_string[position];
        else
            return 0;
    }

    inline void String::clear()
    {
        m_string.clear();
    }

    inline bool String::empty() const
    {
        return m_string.empty();
    }

    inline void String::reserve(UInt capacity)
    {
        m_string.reserve(capacity);
    }

    inline UInt String::capacity() const
    {
        return m_string.capacity();
    }

    inline bool String::startsWith(const String& other) const
    {
        return find(other, 0) == 0;
    }

    inline bool String::endsWith(const String& other) const
    {
        bool result = false;
        UInt ownLen = size();
        UInt otherLen = other.size();
        if (otherLen <= ownLen)
        {
            Int offset = static_cast<Int>(ownLen) - static_cast<Int>(otherLen);
            result = (-1 != find(other, offset));
        }
        return result;
    }

    inline bool String::operator<(const String& other) const
    {
        return m_string < other.m_string;
    }

    inline bool String::operator>(const String& other) const
    {
        return m_string > other.m_string;
    }

    inline Int String::rfind(char ch) const
    {
        const auto res = m_string.rfind(ch);
        if (res == std::string::npos)
            return -1;
        return res;
    }

    inline const std::string& String::stdRef() const
    {
        return m_string;
    }

    inline std::string& String::stdRef()
    {
        return m_string;
    }
}

template <>
struct fmt::formatter<ramses_internal::String>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const ramses_internal::String& str, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", str.stdRef());
    }
};

namespace std
{
    template<>
    struct hash<ramses_internal::String>
    {
        size_t operator()(const ramses_internal::String& key)
        {
            return ramses_capu::HashMemoryRange(key.data(), key.size());
        }
    };
}

#endif
