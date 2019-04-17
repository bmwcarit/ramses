//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORM_STRING_H
#define RAMSES_PLATFORM_STRING_H

#include <ramses-capu/container/String.h>
#include <PlatformAbstraction/PlatformTypes.h>
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"

namespace ramses_internal
{
    class String: public ramses_capu::String
    {
    public:
        String();
        String(const Char* data);
        String(const Char* data, const Int start);
        String(UInt initialSize, const Char character);
        String(const Char* data, const Int start, const Int end);
        String(const String& other) = default;
        String(const ramses_capu::String& other);
        String(String&& other) = default;
        ~String();
        const Char* c_str() const;
        Char at(UInt position) const;
        Int find(const String& substring, const UInt startPos = 0) const;
        String& operator=(const String& other) = default;
        String& operator=(String&& other) = default;
        String& operator=(const Char character);
        String operator+(const String& rOperand) const;
        String operator+(const Char* rOperand) const;
        void operator+=(const Char character);
        void operator+=(const Char* other);
        void operator+=(const String& other);
        Char& operator[](const UInt index);
        Char operator[](const UInt index) const;

        String& operator=(const Char* other);
        Bool operator==(const String& other) const;
        Bool operator==(const char* other) const;
        Bool operator!=(const String& other) const;
        Bool operator!=(const char* other) const;
        String& append(const String& other);
        String& append(const Char* other);
        String substr(UInt start, UInt length) const;
        UInt getLength() const;
        Int indexOf(const Char ch, const UInt startPos = 0) const;
        Int lastIndexOf(const Char ch) const;
        void toUpperCase();
        void toLowerCase();
        String replace(const String& search, const String& replace, UInt startPos = 0) const;
        void clear();
        bool empty() const;

        /**
         * Swaps this string with another
         * @param other The other string
         * @return Reference to this string
         */
        String& swap(String& other);

        const std::string& stdRef() const;
        std::string& stdRef();
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

    inline String::String(const ramses_internal::Char* data)
        : ramses_capu::String(data)
    {
    }

    inline String::String(const ramses_internal::Char* data, const ramses_internal::Int start)
        : ramses_capu::String(data, start)
    {
    }

    inline String::String(const ramses_internal::Char* data, const ramses_internal::Int start, const ramses_internal::Int end)
        : ramses_capu::String(data, start, end)
    {
    }

    inline String::String(const ramses_capu::String& other)
        : ramses_capu::String(other)
    {
    }

    inline String::String(UInt initialSize, const Char character)
        : ramses_capu::String(initialSize, character)
    {
    }

    inline String::~String()
    {
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

    inline String& String::operator=(const ramses_internal::Char* other)
    {
        ramses_capu::String::operator=(other);
        return *this;
    }

    inline Bool String::operator==(const ramses_internal::String& other) const
    {
        return ramses_capu::String::operator==(other);
    }

    inline Bool String::operator==(const char* other) const
    {
        return ramses_capu::String::operator==(other);
    }

    inline Bool String::operator!=(const ramses_internal::String& other) const
    {
        return !operator==(other);
    }

    inline Bool String::operator!=(const char* other) const
    {
        return !operator==(other);
    }

    inline void String::operator+=(const Char character)
    {
        ramses_capu::String::operator+=(character);
    }

    inline void String::operator+=(const Char* other)
    {
        ramses_capu::String::operator+=(other);
    }

    inline void String::operator+=(const String& other)
    {
        ramses_capu::String::operator+=(other.c_str());
    }

    inline Char& String::operator[](const UInt index)
    {
        return ramses_capu::String::operator [](index);
    }

    inline Char String::operator[](const UInt index) const
    {
        return ramses_capu::String::operator [](index);
    }


    inline String& String::operator=(const Char character)
    {
        ramses_capu::String::operator=(character);
        return *this;
    }

    inline String& String::append(const String& other)
    {
        ramses_capu::String::append(other);
        return *this;
    }

    inline void String::toUpperCase()
    {
        ramses_capu::String::toUpperCase();
    }

    inline void String::toLowerCase()
    {
        ramses_capu::String::toLowerCase();
    }

    inline String& String::append(const ramses_internal::Char* other)
    {
        ramses_capu::String::append(other);
        return *this;
    }

    inline String String::substr(UInt start, UInt length) const
    {
        return String(c_str(), start, start + length - 1);
    }

    inline const Char* String::c_str() const
    {
        return ramses_capu::String::c_str();
    }

    inline UInt String::getLength() const
    {
        return ramses_capu::String::getLength();
    }

    inline Int String::indexOf(const ramses_internal::Char ch, const UInt startPos /*= 0*/) const
    {
        return ramses_capu::String::find(ch, startPos);
    }

    inline Int String::lastIndexOf(const ramses_internal::Char ch) const
    {
        return ramses_capu::String::rfind(ch);
    }

    inline String& String::swap(String& other)
    {
        ramses_capu::String::swap(other);
        return *this;
    }

    inline Int String::find(const String& substring, const UInt startPos) const
    {
        if (startPos >= getLength())
        {
            return -1;
        }

        const Char* stringStart = &(c_str()[startPos]);
        const Char* pos = strstr(stringStart, substring.c_str());

        if (pos)
        {
            return (pos - c_str());
        }
        else
        {
            return -1;
        }
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

    inline String String::replace(const String& search, const String& replace, UInt startPos) const
    {
        return ramses_capu::String::replace(search, replace, startPos);
    }

    inline void String::clear()
    {
        truncate(0);
    }

    inline bool String::empty() const
    {
        return getLength() == 0;
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
}
#endif
