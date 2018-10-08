/*
 * Copyright (C) 2012 BMW Car IT GmbH
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

#ifndef RAMSES_CAPU_STRING_H
#define RAMSES_CAPU_STRING_H

#include "ramses-capu/Config.h"
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/container/Hash.h"
#include "ramses-capu/container/vector.h"
#include <algorithm>
#include <cctype>

namespace ramses_capu
{
    /**
     * Character strings.
     */
    class String
    {
    public:

        String();
        /**
         * Create a string from some characters
         * @param data Pointer to characters
         */
        String(const char* data);

        /**
         * Create a string from some characters but not starting from the front
         * @param data Pointer to the characters to copy from
         * @param start Position within characters to start copying from
         */
        String(const char* data, uint_t start);
        /**
         * Create a string from some characters, only taking some from the middle
         * @param data Pointer to character
         * @param start Position within characters to start copying from
         * @param end Position within characters to stop copying
         */
        String(const char* data, const uint_t start, const uint_t end);

        /**
         * Create a string from some other string, only taking some from the middle
         * @param other The string from which a substring will be taken.
         * @param start Position within characters to start copying from
         * @param end Position within characters to stop copying
         */
        String(const String& other, const uint_t start, const uint_t end);

        /**
         * Create a string with initial size and all characters set to the given character
         * @param initialSize for the string
         * @param character to initialize string
         */
        String(uint_t initialSize, char character);

        /**
         * Create a string by copying from another
         */
        String(const String& other) = default;

        /**
         * Move constructor
         */
        String(String&& other) noexcept;

        /**
         * Destructor.
         */
        ~String();

        void resize(uint_t newSize);

        /**
         * Return the string as characters
         */
        const char* c_str() const;

        /**
         * Return pointer to character storage
         */
        const char* data() const;
        char* data();

        /**
         * Assign a string by copying from another
         */
        String& operator=(const String& other) = default;

        /**
         * Assign a string by copying from some characters
         */
        String& operator=(const char* other);

        /**
         * Assign a char to the string
         */
        String& operator=(char other);

        /**
         * Move assignment
         */
        String& operator=(String&& other) noexcept;

        /**
         * Adds the given character string to the string
         * @param character string to add
         */
        void operator+=(const char* other);

        /**
         * Adds the given character to the string
         * @param character string to add
         */
        void operator+=(char other);

        /**
         * Add two strings together and return the concatenated string
         */
        String operator+(const String& rOperand) const;

        /**
         * Concatenate a c-style string and return the result
         */
        String operator+(const char* rOperand) const;

        /**
         * Return if this string equals another
         */
        bool operator==(const String& other) const;

        /**
         * Return if this string equals another
         */
        bool operator==(const char* other) const;

        /**
         * Return if this string does not equalsanother
         */
        bool operator!=(const String& other) const;

        /**
         * Return if this string does not equalsanother
         */
        bool operator!=(const char* other) const;

        /**
         * Return if this string is lexicographically ordered before other
         */
        bool operator<(const String& other) const;

        /**
         * Return if this string is lexicographically ordered after other
         */
        bool operator>(const String& other) const;

        /**
         * Access operator to access a special character
         * @param index of the character to access
         * @return character at the given index
         * @{
         */
        char& operator[](uint_t index);
        const char& operator[](uint_t index) const;
        /**
         * @}
         */

        /**
         * Append the given string to this string
         * @param other The String to append
         * @return Reference to this string
         */
        String& append(const String& other);

        /**
         * Append the given characters to this string
         * @param other The characters to append
         * @return Reference to this string
         */
        String& append(const char* other);

        /**
         * Return the length of the string
         */
        uint_t getLength() const;

        /**
         * Return the first index of the given character within the string
         * @param ch The character whos index is requested
         * @param offset The index from where the search for the character has to be started (default 0).
         * @return The index of the found char or -1 if the char was not found.
         */
        int_t find(const char ch, const uint_t offset = 0) const;

        /**
         * Return the first index of the given substring within the string
         * @param substr The substring whos index is requested
         * @param offset The index from where the search for the substring has to be started (default 0).
         * @return The index of the found substring or -1 if the substring was not found.
         */
        int_t find(const String& substr, const uint_t offset = 0) const;

        /**
         * Checks if the String starts with the given string
         * @param other string to check
         * @return true if String starts with other string. False otherwise
         */
        bool startsWith(const String& other) const;

        /**
         * Checks if the String ends with the given string
         * @param other string to check
         * @return true if String ends with other string. False otherwise
         */
        bool endsWith(const String& other) const;

        /**
         * Return the index of the last occurence of the given character within the string
         * @param ch The character whos last index is requested
         */
        int_t rfind(const char ch) const;

        /**
         * Convert the string to UPPER CASE
         */
        void toUpperCase();

        /**
         * Convert the string to lower case
         */
        void toLowerCase();

        /**
         * Swaps this string with another
         * @param other The other string
         * @return Reference to this string
         */
        String& swap(String& other);

        /**
         * Truncated the string to the given length.
         *
         * If the length ist equal or greater than the size of the string
         * this is a no-op. Otherwise the string wil be shortened.
         * In order to avoid copying truncating runs inplace so the buffer
         * size won't get changed.
         *
         * @param length the new length of the string
         * @return Reference to this string.
         */
        String& truncate(uint_t length);

        /**
         * Extracts a substring of the string with the given start and length.
         *
         * If the length exceeds the string last character, it is set to match the
         * string length. If the length is <0 the method returns the remainder of the
         * string from the given start.
         *
         * @param start the start character position
         * @param length the length of the substring
         * @return The substring.
         */
        String substr(uint_t start, int_t length) const;

        /**
         * Replaces a substring within the string with a substitutionary string, starting at a given character position.
         * @param search the substring to be replaced
         * @param replace the substring, which is to be put in place for the search substring
         * @param offset the start character position for starting the replacement.
         * @return string The string with replaced search substring.
         */
        String replace(const String& search, const String& replace, const int_t offset = 0) const;

        void reserve(uint_t capacity);
        uint_t capacity() const;

    private:
        void initData(const char* data);
        void initFromGivenData(const char* data, const uint_t end, const uint_t start, uint_t size);
        String& appendWithKnownLength(const char* other, uint_t length);

        vector<char> m_data;
        uint_t m_size;
    };

    static_assert(std::is_nothrow_move_constructible<String>::value &&
                  std::is_nothrow_move_assignable<String>::value, "String must be movable");

    /**
     * Specialization of Hash in order to calculate the Hash differently for strings
     */
    template<>
    struct Hash<String>
    {
        uint_t operator()(const String& key)
        {
            return HashMemoryRange(key.data(), key.getLength());
        }
    };

    /**
     * Overloading swap for String
     */
    inline void swap(String& first, String& second)
    {
        first.swap(second);
    }

    // global comparison
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
        : m_data(), m_size(0)
    {
    }

    inline String::String(uint_t initialSize, char character)
        : m_data(initialSize + 1, character)
        , m_size(initialSize)
    {
        m_data[initialSize] = '\0';
    }

    inline String::String(const char* other)
        : m_data(), m_size(0)
    {
        initData(other);
    }

    inline String::String(const char* data, uint_t start)
        : m_data(), m_size(0)
    {
        if (data)
        {
            const char* startdata = &data[start];
            initData(startdata);
        }
    }

    inline String::String(const String& other, const uint_t start, const uint_t end)
        : m_data(), m_size(0)
    {
        initFromGivenData(other.c_str(), start, end, other.m_size);
    }

    inline String::String(const char* data, const uint_t start, const uint_t end)
        : m_data(), m_size(0)
    {
        initFromGivenData(data, start, end, StringUtils::Strnlen(data, end + 1));
    }

    inline String::String(String&& other) noexcept
        : m_data(std::move(other.m_data))
        , m_size(other.m_size)
    {
        other.m_size = 0;
    }

    inline void String::initFromGivenData(const char* data, const uint_t start, const uint_t end, uint_t size)
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
        uint_t theend = end;
        if (theend >= size)
        {
            theend = size - 1;
        }

        // do the work
        const char* startdata = data + start;
        m_size = theend - start + 1;

        m_data.reserve(m_size + 1);
        m_data.clear();
        m_data.insert(m_data.end(), startdata, startdata + m_size);
        m_data.push_back('\0');
    }

    inline String::~String()
    {
    }

    inline void String::resize(uint_t newSize)
    {
        if (newSize>0)
        {
            m_data.resize(newSize + 1); // additional byte for null termination
            m_data[newSize] = 0;
        }
        else
        {
            m_data.resize(0);
        }
        m_size = newSize;
    }

    inline String& String::operator=(const char* other)
    {
        initData(other);
        return *this;
    }

    inline String& String::operator=(String&& other) noexcept
    {
        m_data = std::move(other.m_data);
        m_size = other.m_size;
        other.m_size = 0;
        return *this;
    }

    inline String String::operator+(const String& rOperand) const
    {
        String result(*this);
        return result.append(rOperand);
    }

    inline String String::operator+(const char* rOperand) const
    {
        String result(c_str());
        return result.append(rOperand);
    }

    inline String operator+(const char* lOperand, const String& rOperand)
    {
        String result(lOperand);
        return result.append(rOperand);
    }

    inline bool String::operator==(const String& other) const
    {
        const char null(0);
        const char* str1 = m_data.data();
        if (!str1)
        {
            str1 = &null;
        }
        const char* str2 = other.m_data.data();
        if (!str2)
        {
            str2 = &null;
        }
        return StringUtils::Strcmp(str1, str2) == 0;
    }

    inline bool String::operator==(const char* other) const
    {
        const char null(0);
        const char* str1 = m_data.data();
        if (!str1)
        {
            str1 = &null;
        }
        if (!other)
        {
            other = &null;
        }
        return StringUtils::Strcmp(str1, other) == 0;
    }

    inline bool String::operator<(const String& other) const
    {
        const char null(0);
        const char* str1 = m_data.data();
        if (!str1)
        {
            str1 = &null;
        }
        const char* str2 = other.m_data.data();
        if (!str2)
        {
            str2 = &null;
        }
        return StringUtils::Strcmp(str1, str2) < 0;
    }

    inline bool String::operator>(const String& other) const
    {
        const char null(0);
        const char* str1 = m_data.data();
        if (!str1)
        {
            str1 = &null;
        }
        const char* str2 = other.m_data.data();
        if (!str2)
        {
            str2 = &null;
        }
        return StringUtils::Strcmp(str1, str2) > 0;
    }

    inline void String::operator+=(const char* other)
    {
        append(other);
    }

    inline String& String::operator=(char other)
    {
        char tmp[2] = {other, '\0'};
        return operator=(tmp);
    }

    inline void String::operator+=(char other)
    {
        char tmp[2] = {other, '\0'};
        operator+=(tmp);
    }

    inline bool String::operator!=(const String& other) const
    {
        return !operator==(other);
    }

    inline bool String::operator!=(const char* other) const
    {
        return !operator==(other);
    }

    inline char& String::operator[](uint_t index)
    {
        return m_data[index];
    }

    inline const char& String::operator[](uint_t index) const
    {
        return m_data[index];
    }

    inline String& String::append(const String& other)
    {
        return appendWithKnownLength(other.data(), other.getLength());
    }

    inline void String::toUpperCase()
    {
        const uint_t length = getLength();
        for (uint_t i = 0; i < length; ++i)
        {
            m_data[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(m_data[i])));
        }
    }

    inline void String::toLowerCase()
    {
        const uint_t length = getLength();
        for (uint_t i = 0; i < length; ++i)
        {
            m_data[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(m_data[i])));
        }
    }

    inline String& String::append(const char* other)
    {
        if (other && *other)
        {
            const uint_t otherlen = StringUtils::Strlen(other);
            return appendWithKnownLength(other, otherlen);
        }
        return *this;
    }

    inline String& String::appendWithKnownLength(const char* other, uint_t otherLength)
    {
        if (m_data.data() || other)
        {
            m_data.reserve(m_size + otherLength + 1);
            m_data.resize(m_size);
            m_data.insert(m_data.end(), other, other + otherLength);
            m_data.push_back('\0');
            m_size += otherLength;
        }
        return *this;
    }

    inline const char* String::c_str() const
    {
        return m_data.size() > 0 ? m_data.data() : "";
    }

    inline const char* String::data() const
    {
        return m_data.data();
    }

    inline char* String::data()
    {
        return m_data.data();
    }

    inline void String::initData(const char* data)
    {
        if (data && *data)
        {
            m_size = StringUtils::Strlen(data);
            m_data.clear();
            m_data.reserve(m_size + 1);
            m_data.insert(m_data.end(), data, data + m_size);
            m_data.push_back('\0');
        }
        else
        {
            m_data.clear();
            m_size = 0;
        }
    }

    inline uint_t String::getLength() const
    {
        return m_size;
    }

    inline int_t String::find(const char ch, const uint_t offset) const
    {
        return StringUtils::IndexOf(c_str(), ch, offset);
    }

    inline int_t String::find(const String& substr, const uint_t offset) const
    {
        return StringUtils::IndexOf(c_str(), substr.c_str(), offset);;
    }

    inline int_t String::rfind(const char ch) const
    {
        return StringUtils::LastIndexOf(c_str(), ch);
    }

    inline String& String::swap(String& other)
    {
        m_data.swap(other.m_data);
        using std::swap;
        swap(m_size, other.m_size);
        return *this;
    }

    inline String& String::truncate(uint_t length)
    {
        if (length >= getLength())
        {
            // nothing to do
            return *this;
        }

        // set new size and append null char after end of string
        m_size = length;
        m_data.resize(length + 1);
        m_data[length] = 0;
        return *this;
    }

    inline String String::substr(uint_t start, int_t length) const
    {
        if (length < 0 || static_cast<uint_t>(length) > m_size)
        {
            // take the complete substring starting with start
            return String(*this, start, m_size);
        }
        else
        {
            if (length == 0)
            {
                return String("");
            }
            // calculate end position and cut out substring
            return String(*this, start, start + length - 1);
        }
    }

    inline
    bool
    String::startsWith(const String& other) const
    {
        return find(other, 0) == 0;
    }

    inline
    bool
    String::endsWith(const String& other) const
    {
        bool result = false;
        uint_t ownLen = getLength();
        uint_t otherLen = other.getLength();
        if (otherLen <= ownLen)
        {
            int_t offset = static_cast<int_t>(ownLen) - static_cast<int_t>(otherLen);
            result = (-1 != find(other, offset));
        }
        return result;
    }

    inline
        String
        String::replace(const String& search, const String& replace, const int_t offset) const
    {
        String result = substr(0, offset);
        const int_t searchLen = search.getLength();
        int_t nextPos;
        int_t lastPos = offset;
        while ((nextPos = find(search, lastPos)) > -1)
        {
            result = result + substr(lastPos, nextPos - lastPos);
            result = result + replace;
            lastPos = nextPos + searchLen;
        }
        return result + substr(lastPos, -1);
    }

    inline
    void String::reserve(uint_t capacity)
    {
        m_data.reserve(capacity + 1);
    }

    inline
    uint_t String::capacity() const
    {
        return m_data.capacity() == 0 ? 0u : m_data.capacity() - 1;
    }
}

#endif // RAMSES_CAPU_STRING_H
