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
#include "PlatformAbstraction/Hash.h"
#include "PlatformAbstraction/FmtBase.h"
#include "Utils/AssertMovable.h"
#include "absl/strings/string_view.h"
#include <string>
#include <cctype>

namespace ramses_internal
{
    class String final
    {
    public:
        String() = default;
        String(const Char* data);   // NOLINT(google-explicit-constructor) we want implicit conversion compatible to std::string
        String(UInt initialSize, Char character);
        String(const Char* data, UInt start, UInt end);

        explicit String(const std::string& other);
        explicit String(std::string&& other);
        explicit String(absl::string_view sv);

        String(const String& other) = default;
        String(String&& other) noexcept = default;
        ~String() = default;
        RNODISCARD const Char* c_str() const;
        RNODISCARD Char at(UInt position) const;
        RNODISCARD size_t find(const String& substring, size_t pos = 0) const;
        RNODISCARD size_t find(char ch, size_t pos = 0) const;
        RNODISCARD size_t rfind(char ch) const;

        String& operator=(const String& other) = default;
        String& operator=(String&& other) noexcept = default;
        String& operator=(const std::string& other);
        String& operator=(std::string&& other);
        String& operator=(Char character);
        String& operator=(const Char* other);

        RNODISCARD String operator+(const String& rOperand) const;
        RNODISCARD String operator+(const std::string& rOperand) const;
        RNODISCARD String operator+(const Char* rOperand) const;
        void operator+=(Char character);
        void operator+=(const Char* other);
        void operator+=(const String& other);

        RNODISCARD Char& operator[](UInt index);
        RNODISCARD Char operator[](UInt index) const;

        RNODISCARD bool operator==(const String& other) const;
        RNODISCARD bool operator==(const std::string& other) const;
        RNODISCARD bool operator==(const char* other) const;
        RNODISCARD bool operator==(absl::string_view other) const;
        RNODISCARD bool operator!=(const String& other) const;
        RNODISCARD bool operator!=(const std::string& other) const;
        RNODISCARD bool operator!=(const char* other) const;
        RNODISCARD bool operator!=(absl::string_view other) const;

        String& append(const String& other);
        String& append(const std::string& other);
        String& append(const Char* other);

        RNODISCARD String substr(size_t start, size_t length) const;
        RNODISCARD UInt size() const;
        void clear();
        RNODISCARD bool empty() const;
        void resize(UInt newSize);
        RNODISCARD char* data();
        RNODISCARD const char* data() const;
        void reserve(UInt capacity);
        RNODISCARD UInt capacity() const;
        RNODISCARD bool operator<(const String& other) const;
        RNODISCARD bool operator>(const String& other) const;

        operator absl::string_view() const noexcept;  // NOLINT(google-explicit-constructor) implicit conversion as in std::string

        /**
         * Swaps this string with another
         * @param other The other string
         * @return Reference to this string
         */
        String& swap(String& other);

        RNODISCARD const std::string& stdRef() const;
        RNODISCARD std::string& stdRef();

        static constexpr auto npos = std::string::npos;

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

    ASSERT_MOVABLE(String)

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

    inline bool operator==(absl::string_view a, const String& b)
    {
        return b == a;
    }

    inline bool operator!=(absl::string_view a, const String& b)
    {
        return b != a;
    }

    /*
     * Implementation String
     */

    inline String::String(const Char* data)
    {
        if (data != nullptr && *data != 0)
            m_string = data;
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

    inline String::String(absl::string_view sv)
        : m_string(sv)
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

    inline bool String::operator==(absl::string_view other) const
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

    inline bool String::operator!=(absl::string_view other) const
    {
        return m_string != other;
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
        operator=(tmp);
        return *this;
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

    inline String& String::append(const Char* other)
    {
        if (other)
            m_string += other;
        return *this;
    }

    inline String String::substr(size_t start, size_t length) const
    {
        // we allow start and start+length to exceed size
        const size_t curSize = size();
        return String(m_string.substr(std::min(start, curSize),
                                      std::min(length, curSize - start)));
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

    inline size_t String::find(const String& substring, size_t startPos) const
    {
        return m_string.find(substring.m_string, startPos);
    }

    inline size_t String::find(char ch, size_t startPos) const
    {
        return m_string.find(ch, startPos);
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

    inline bool String::operator<(const String& other) const
    {
        return m_string < other.m_string;
    }

    inline bool String::operator>(const String& other) const
    {
        return m_string > other.m_string;
    }

    inline size_t String::rfind(char ch) const
    {
        return m_string.rfind(ch);
    }

    inline const std::string& String::stdRef() const
    {
        return m_string;
    }

    inline std::string& String::stdRef()
    {
        return m_string;
    }

    inline String::operator absl::string_view() const noexcept
    {
        return m_string;
    }
}

// disable formatter for c++17 and up to prevent ambiguity between this formatter and formatting
// via implicit conversion from String to string_view.
#if __cplusplus < 201703L
template <>
struct fmt::formatter<ramses_internal::String> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::String& str, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "{}", str.stdRef());
    }
};
#endif

template<>
struct std::hash<ramses_internal::String>
{
    size_t operator()(const ramses_internal::String& key)
    {
        return ramses_internal::HashMemoryRange(key.data(), key.size());
    }
};

#endif
