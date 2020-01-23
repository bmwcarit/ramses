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
    class Matrix22f;
    class Matrix33f;
    class Matrix44f;

    class StringOutputStream final
    {
    public:
        enum EFloatingPointType
        {
            EFloatingPointType_Normal = 0,
            EFloatingPointType_Fixed
        };

        enum EHexadecimalType
        {
            EHexadecimalType_NoHex = 0,
            EHexadecimalType_HexNoLeadingZeros,
            EHexadecimalType_HexLeadingZeros
        };

        explicit StringOutputStream(UInt initialCapacity = 16);
        explicit StringOutputStream(String initialContent);
        explicit StringOutputStream(std::string initialContent);

        StringOutputStream& operator<<(unsigned short value);
        StringOutputStream& operator<<(unsigned int value);
        StringOutputStream& operator<<(unsigned long value);
        StringOutputStream& operator<<(unsigned long long value);
        StringOutputStream& operator<<(signed short value);
        StringOutputStream& operator<<(signed int value);
        StringOutputStream& operator<<(signed long value);
        StringOutputStream& operator<<(signed long long value);

        StringOutputStream& operator<<(const String& value);
        StringOutputStream& operator<<(const std::string& value);
        StringOutputStream& operator<<(const bool value);
        StringOutputStream& operator<<(const Float value);
        StringOutputStream& operator<<(const Char* value);
        StringOutputStream& operator<<(const Char value);
        StringOutputStream& operator<<(const void* value);
        StringOutputStream& operator<<(const Matrix22f& value);
        StringOutputStream& operator<<(const Matrix33f& value);
        StringOutputStream& operator<<(const Matrix44f& value);

        void clear();

        void reserve(UInt capacity);
        UInt capacity() const;

        const Char* c_str() const;
        UInt32 size() const;

        String release();
        const String& data() const;

        void setFloatingPointType(EFloatingPointType type);
        void setDecimalDigits(int digits);
        void setHexadecimalOutputFormat(EHexadecimalType hexFormat);

        template <typename T>
        static String ToString(const T& value);

    private:
        template <typename MatrixType>
        StringOutputStream& outputMatrix(const MatrixType& matrix);
        StringOutputStream& write(const char* data, uint32_t size);

        template <typename T>
        StringOutputStream& writeUnsigned(T value)
        {
            static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "wrong type");

            char buffer[21];
            switch(m_hexadecimalFormat)
            {
            case EHexadecimalType_HexNoLeadingZeros:
                std::snprintf(buffer, sizeof(buffer), "%" PRIX64, static_cast<uint64_t>(value));
                break;
            case EHexadecimalType_HexLeadingZeros:
                // 2 leading zeros per byte in T
                std::snprintf(buffer, sizeof(buffer), "%0*" PRIX64, static_cast<int>(sizeof(T)*2), static_cast<uint64_t>(value));
                break;
            case EHexadecimalType_NoHex:
            default:
                std::snprintf(buffer, sizeof(buffer), "%" PRIu64, static_cast<uint64_t>(value));
                break;
            }
            return operator<<(buffer);
        }

        template <typename T>
        StringOutputStream& writeSigned(T value)
        {
            static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "wrong type");

            if (m_hexadecimalFormat == EHexadecimalType_HexNoLeadingZeros ||
                m_hexadecimalFormat == EHexadecimalType_HexLeadingZeros)
            {
                // fall back to unsigned for hex printing
                return writeUnsigned(static_cast<std::make_unsigned_t<T>>(value));
            }
            else
            {
                char buffer[21];
                std::snprintf(buffer, sizeof(buffer), "%" PRId64, static_cast<int64_t>(value));
                return operator<<(buffer);
            }
        }

        String m_buffer;
        EFloatingPointType m_floatingPointType = EFloatingPointType_Normal;
        EHexadecimalType m_hexadecimalFormat = EHexadecimalType_NoHex;
        int m_decimalDigits = 6;
    };

    inline
    StringOutputStream::StringOutputStream(UInt initialCapacity)
    {
        m_buffer.reserve(initialCapacity);
    }

    inline
    StringOutputStream::StringOutputStream(String initialContent)
        : m_buffer(std::move(initialContent))
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

    inline StringOutputStream& StringOutputStream::operator<<(unsigned short value)
    {
        return writeUnsigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(unsigned int value)
    {
        return writeUnsigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(unsigned long value)
    {
        return writeUnsigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(unsigned long long value)
    {
        return writeUnsigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(signed short value)
    {
        return writeSigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(signed int value)
    {
        return writeSigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(signed long value)
    {
        return writeSigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(signed long long value)
    {
        return writeSigned(value);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const String& value)
    {
        return write(value.c_str(), static_cast<uint32_t>(value.size()));
    }

    inline StringOutputStream& StringOutputStream::operator<<(const std::string& value)
    {
        return write(value.c_str(), static_cast<uint32_t>(value.size()));
    }

    inline StringOutputStream& StringOutputStream::operator<<(const bool value)
    {
        return *this << (value ? "true" : "false");
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Float value)
    {
        /* Maximum length float value
           biggest/smallest: ~ +/-1e38
           closest to zero:  ~ 1e-38

           required maximum buffer length
           - 1 sign
           - 38 digits left of .
           - 1 for .
           - 38 digits right of dot + maximum 7 more relevant digits
           - 1 for terminating \0
           => 1+38+1+38+7+1 = 86
        */
        char buffer[86];
        switch(m_floatingPointType)
        {
        case EFloatingPointType_Normal:
            std::snprintf(buffer, sizeof(buffer), "%.*f", m_decimalDigits, value);
            break;
        case EFloatingPointType_Fixed:
            std::snprintf(buffer, sizeof(buffer), "%.4f", value);
            break;
        }

        return operator<<(buffer);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Char* value)
    {
        if (value)
            return write(value, static_cast<uint32_t>(std::strlen(value)));
        return *this;
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Char value)
    {
        return write(&value, 1);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const void* pointer)
    {
        Char buffer[17];  // 64bit pointer => 16 hex digits + null
        std::snprintf(buffer, sizeof(buffer), "%p", pointer);
        return *this << buffer;
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
    void StringOutputStream::setFloatingPointType(EFloatingPointType type)
    {
        m_floatingPointType = type;
    }

    inline
    void StringOutputStream::setHexadecimalOutputFormat(EHexadecimalType type)
    {
        m_hexadecimalFormat = type;
    }

    inline
    void StringOutputStream::setDecimalDigits(int digits)
    {
        if (digits > 45)
        {
            // limit to maximum useful range for IEEE-754 single precision floats
            m_decimalDigits = 45;
        }
        else
        {
            m_decimalDigits = digits;
        }
    }

    inline
    StringOutputStream& StringOutputStream::write(const char* data, uint32_t size)
    {
        const UInt writeIdx = m_buffer.size();
        m_buffer.resize(m_buffer.size() + size);
        ramses_capu::Memory::Copy(&m_buffer[writeIdx], data, size);
        return *this;
    }

    inline
    const String& StringOutputStream::data() const
    {
        return m_buffer;
    }

    inline
    String StringOutputStream::release()
    {
        String tmp;
        tmp.swap(m_buffer);
        return tmp;
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
