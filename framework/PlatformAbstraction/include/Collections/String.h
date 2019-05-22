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
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/container/Hash.h"
#include <string>
#include <cctype>

namespace ramses_internal
{
    class String
    {
    public:
        String();
        String(const Char* data);
        String(const Char* data, Int start);
        String(UInt initialSize, Char character);
        String(const Char* data, Int start, Int end);
        String(const std::string& other);
        String(std::string&& other);
        String(const String& other) = default;
        String(String&& other) = default;
        ~String();
        const Char* c_str() const;
        Char at(UInt position) const;
        Int find(const String& substring, UInt startPos = 0) const;
        Int find(char ch, UInt offset = 0) const;
        String& operator=(const String& other) = default;
        String& operator=(String&& other) = default;
        String& operator=(Char character);
        String operator+(const String& rOperand) const;
        String operator+(const Char* rOperand) const;
        void operator+=(Char character);
        void operator+=(const Char* other);
        void operator+=(const String& other);
        Char& operator[](UInt index);
        Char operator[](UInt index) const;

        String& operator=(const Char* other);
        Bool operator==(const String& other) const;
        Bool operator==(const char* other) const;
        Bool operator!=(const String& other) const;
        Bool operator!=(const char* other) const;
        String& append(const String& other);
        String& append(const Char* other);
        String substr(UInt start, Int length) const;
        UInt getLength() const;
        Int indexOf(Char ch, UInt startPos = 0) const;
        Int lastIndexOf(Char ch) const;
        void toUpperCase();
        void toLowerCase();
        void clear();
        bool empty() const;
        void resize(UInt newSize);
        char* data();
        const char* data() const;
        String& truncate(UInt length);
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
        void initFromGivenData(const char* data, UInt start, UInt end, UInt size);

        std::string m_string;
    };

    inline IOutputStream& operator<<(IOutputStream& stream, const String& value)
    {
        const uint32_t len = static_cast<uint32_t>(value.getLength());
        stream << len;
        return stream.write(value.c_str(), len);
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

    static_assert(std::is_nothrow_move_constructible<String>::value &&
                  std::is_nothrow_move_assignable<String>::value, "String must be movable");

    // free comparison functions
    inline bool operator==(const char* a, const String& b)
    {
        return b == a;
    }

    inline bool operator!=(const char* a, const String& b)
    {
        return b != a;
    }

    /*
     * Implementation String
     */

    inline String::String()
    {
    }

    inline String::String(const Char* other)
    {
        if (other && *other)
            m_string = other;
    }

    inline String::String(const Char* data, Int start)
    {
        if (data)
            m_string = data + start;
    }

    inline String::String(const Char* data, Int start, Int end)
    {
        initFromGivenData(data, start, end, ramses_capu::StringUtils::Strnlen(data, end + 1));
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

    inline String::~String()
    {
    }

    inline void String::initFromGivenData(const char* data, UInt start, UInt end, UInt size)
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

    inline Bool String::operator==(const String& other) const
    {
        return m_string == other.m_string;
    }

    inline Bool String::operator==(const char* other) const
    {
        return m_string == other;
    }

    inline Bool String::operator!=(const String& other) const
    {
        return !operator==(other);
    }

    inline Bool String::operator!=(const char* other) const
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

    inline UInt String::getLength() const
    {
        return m_string.size();
    }

    inline Int String::indexOf(Char ch, UInt startPos /*= 0*/) const
    {
        return ramses_capu::StringUtils::IndexOf(c_str(), ch, startPos);
    }

    inline Int String::lastIndexOf(Char ch) const
    {
        return ramses_capu::StringUtils::LastIndexOf(c_str(), ch);
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
        if (position < getLength())
        {
            return c_str()[position];
        }
        else
        {
            return 0;
        }
    }

    inline void String::clear()
    {
        truncate(0);
    }

    inline String& String::truncate(UInt length)
    {
        if (length >= getLength())
        {
            // nothing to do
            return *this;
        }

        m_string.resize(length);
        return *this;
    }

    inline bool String::empty() const
    {
        return getLength() == 0;
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
        UInt ownLen = getLength();
        UInt otherLen = other.getLength();
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
        return ramses_capu::StringUtils::LastIndexOf(c_str(), ch);
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

namespace ramses_capu
{
    template<>
    struct Hash<ramses_internal::String>
    {
        uint_t operator()(const ramses_internal::String& key)
        {
            return HashMemoryRange(key.data(), key.getLength());
        }
    };

    template<>
    struct Hash<std::string>
    {
        uint_t operator()(const std::string& key)
        {
            return HashMemoryRange(key.data(), key.size());
        }
    };
}

#endif
