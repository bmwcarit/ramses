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

#ifndef RAMSES_CAPU_CONSTSTRING_H
#define RAMSES_CAPU_CONSTSTRING_H

#include <ramses-capu/Config.h>
#include <ramses-capu/os/StringUtils.h>

namespace ramses_capu
{
    /**
     * Implements a const string which is not mutable
     * No data is copied to the string
     */
    class ConstString
    {
    public:
        /**
         * Constructs a string from a const char pointer
         * @param str which points to a const char
         */
        ConstString(const char* str);

        /**
         * Create a string from some characters but not starting from the front
         * @param data Pointer to the characters
         * @param start Position within characters to start
         */
        ConstString(const char* data, const uint_t start);

        /**
         * Creates a ConstString from another ConstString
         * @param other string to construct from
         */
        ConstString(const ConstString& other);

        /**
         * Returns the internal const char pointer
         * @return the internal const char pointer
         */
        const char* c_str() const;

        /**
         * Calculates and returns the length of the string. This operation is
         * expensive.
         * @return the length of the string
         */
        uint_t length() const;

        /**
         * Converts a ConstString to a const char pointer
         */
        operator const char* () const;

        /**
         * Compares two ConstStrings
         * @param other string to compare with
         * @return true if both strings are identical, false otherwise
         */
        bool operator==(const ConstString& other) const;

        /**
         * Compares two ConstStrings
         * @param other string to compare with
         * @return true if this string is less than the other string, false otherwise
         */
        bool operator<(const ConstString& other) const;

        /**
         * Compares two ConstStrings
         * @param other string to compare with
         * @return true if this string is greater than the other string, false otherwise
         */
        bool operator>(const ConstString& other) const;

        /**
         * Return if this string does not equals other
         * @return true if this ConsTring does not equal other string, false otherwise
         */
        bool operator!=(const ConstString& other) const;

        /**
         * Assignment operator for ConstString
         * @param other string to assign
         * @return reference to this string
         */
        ConstString& operator=(const ConstString& other);

        /**
         * Return the first index of the given character within the ConstString
         * @param ch The character whos index is requested
         * @param offset The index from where the search for the character has to be started (default 0).
         * @return The index of the found char or -1 if the char was not found.
         */
        int_t find(const char ch, const uint_t offset = 0) const;

        /**
         * Return the first index of the given substring within the ConstString
         * @param substr The substring whos index is requested
         * @param offset The index from where the search for the substring has to be started (default 0).
         * @return The index of the found substring or -1 if the substring was not found.
         */
        int_t find(const ConstString& substr, const uint_t offset = 0) const;

        /**
         * Return the index of the last occurence of the given character within the ConstString
         * @param ch The character whos last index is requested
         */
        int_t rfind(const char ch) const;

    protected:
    private:
        /**
         * Internal const char* to work with
         */
        const char* m_data;
    };

    inline
    ConstString::ConstString(const char* str)
        : m_data(str)
    {
    }

    inline
    ConstString::ConstString(const ConstString& other)
        : m_data(other.c_str())
    {
    }

    inline
    ConstString::ConstString(const char* data, const uint_t start)
        : m_data(0)
    {
        if (0 != data)
        {
            const char* startdata = &data[start];
            m_data = startdata;
        }
    }

    inline
    const char*
    ConstString::c_str() const
    {
        return m_data;
    }

    inline
    uint_t
    ConstString::length() const
    {
        return StringUtils::Strlen(m_data);
    }

    inline
    ConstString::operator const char* () const
    {
        return c_str();
    }

    inline
    bool
    ConstString::operator==(const ConstString& other) const
    {
        return StringUtils::Strcmp(m_data, other.m_data) == 0;
    }

    inline
    bool
    ConstString::operator!=(const ConstString& other) const
    {
        return !operator==(other);
    }

    inline
    bool ConstString::operator<(const ConstString& other) const
    {
        return StringUtils::Strcmp(m_data, other.m_data) < 0;
    }

    inline
    bool ConstString::operator>(const ConstString& other) const
    {
        return StringUtils::Strcmp(m_data, other.m_data) > 0;
    }

    inline
    ConstString&
    ConstString::operator=(const ConstString& other)
    {
        m_data = other.m_data;
        return *this;
    }

    inline
    int_t
    ConstString::find(const char ch, const uint_t offset) const
    {
        return StringUtils::IndexOf(c_str(), ch, offset);
    }

    inline
    int_t
    ConstString::find(const ConstString& substr, const uint_t offset) const
    {
        return StringUtils::IndexOf(c_str(), substr, offset);
    }

    inline
    int_t
    ConstString::rfind(const char ch) const
    {
        return StringUtils::LastIndexOf(c_str(), ch);
    }
}

#endif
