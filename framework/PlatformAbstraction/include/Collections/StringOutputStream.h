//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSES_STRINGOUTPUTSTREAM_H
#define RAMSES_RAMSES_STRINGOUTPUTSTREAM_H

#include "PlatformAbstraction/PlatformStringUtils.h"
#include "Collections/Guid.h"
#include "Collections/String.h"
#include "Collections/Vector.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Utils/StringUtils.h"

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

        StringOutputStream& operator<<(const Int32 value);
        StringOutputStream& operator<<(const UInt32 value);
        StringOutputStream& operator<<(const UInt64 value);
        StringOutputStream& operator<<(const Int64 value);
        StringOutputStream& operator<<(const String& value);
        StringOutputStream& operator<<(const Bool value);
        StringOutputStream& operator<<(const Float value);
        StringOutputStream& operator<<(const Char* value);
        StringOutputStream& operator<<(const Char value);
        StringOutputStream& operator<<(const void* value);
        StringOutputStream& operator<<(const UInt16 value);
        StringOutputStream& operator<<(const Int16 value);
        StringOutputStream& operator<<(const Guid& guid);
        StringOutputStream& operator<<(const Matrix22f& value);
        StringOutputStream& operator<<(const Matrix33f& value);
        StringOutputStream& operator<<(const Matrix44f& value);
        StringOutputStream& operator<<(const ResourceContentHash& value);

        void clear();

        void reserve(UInt capacity);
        UInt capacity() const;

        const Char* c_str() const;
        UInt32 length() const;

        String release();

        void setFloatingPointType(EFloatingPointType type);
        void setDecimalDigits(UInt32 digits);
        void setHexadecimalOutputFormat(EHexadecimalType hexFormat);

    private:
        template <typename MatrixType>
        StringOutputStream& outputMatrix(const MatrixType& matrix);
        StringOutputStream& write(const char* data, uint32_t size);

        String m_buffer;
        EFloatingPointType m_floatingPointType;
        EHexadecimalType m_hexadecimalFormat;
        uint32_t m_decimalDigits;
    };

    inline
    StringOutputStream::StringOutputStream(UInt initialCapacity)
        : m_floatingPointType(EFloatingPointType_Normal)
        , m_hexadecimalFormat(EHexadecimalType_NoHex)
        , m_decimalDigits(6)
    {
        m_buffer.reserve(initialCapacity);
    }

    inline
    UInt32 StringOutputStream::length() const
    {
        return static_cast<uint32_t>(m_buffer.getLength());
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Int32 value)
    {
        char buffer[12];

        if(m_hexadecimalFormat != EHexadecimalType_NoHex)
        {
            uint32_t conv = static_cast<uint32_t>(value);
            return operator<<(conv);
        }
        else
        {
            ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%d", value);
            return operator<<(buffer);
        }
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Int64 value)
    {
        char buffer[21];

        if(m_hexadecimalFormat != EHexadecimalType_NoHex)
        {
            uint64_t conv = static_cast<uint64_t>(value);
            return operator<<(conv);
        }
        else
        {
            ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%lld", value);
            return operator<<(buffer);
        }
    }

    inline StringOutputStream& StringOutputStream::operator<<(const UInt64 value)
    {
        char buffer[21];

        switch(m_hexadecimalFormat)
        {
            case EHexadecimalType_HexNoLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%llX", value);
                break;
            case EHexadecimalType_HexLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%016llX", value);
                break;
            case EHexadecimalType_NoHex:
            default:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%llu", value);
                break;
        }

        return operator<<(buffer);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const UInt32 value)
    {
        char buffer[11];

        switch(m_hexadecimalFormat)
        {
            case EHexadecimalType_HexNoLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%X", value);
                break;
            case EHexadecimalType_HexLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%08X", value);
                break;
            case EHexadecimalType_NoHex:
            default:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%u", value);
                break;
        }

        return operator<<(buffer);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const String& value)
    {
        return write(value.c_str(), static_cast<uint32_t>(value.getLength()));
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
            ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%.*f", m_decimalDigits, value);
            break;
        case EFloatingPointType_Fixed:
            ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%.4f", value);
            break;
        }

        return operator<<(buffer);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Char* value)
    {
        return write(value, static_cast<uint32_t>(ramses_capu::StringUtils::Strlen(value)));
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Char value)
    {
        return write(&value, 1);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const void* pointer)
    {
        static const UInt32 BytesForNullTerminator = 1;
        static const UInt32 BytesPerHexDigit = 2;
        static const UInt32 TotalSizeOfString = sizeof(void*)*BytesPerHexDigit + BytesForNullTerminator;

        Char buffer[TotalSizeOfString];
        PlatformStringUtils::Sprintf(buffer, TotalSizeOfString, "%p", pointer);
        return *this << buffer;
    }

    inline StringOutputStream& StringOutputStream::operator<<(const UInt16 value)
    {
        char buffer[7];
        switch(m_hexadecimalFormat)
        {
            case EHexadecimalType_HexNoLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%hX", value);
                break;
            case EHexadecimalType_HexLeadingZeros:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "0x%04hX", value);
                break;
            case EHexadecimalType_NoHex:
            default:
                ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%u", value);
                break;
        }

        return operator<<(buffer);
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Int16 value)
    {
        char buffer[7];

        if(m_hexadecimalFormat != EHexadecimalType_NoHex)
        {
            uint16_t conv = static_cast<uint16_t>(value);
            return operator<<(conv);
        }
        else
        {
            ramses_capu::StringUtils::Sprintf(buffer, sizeof(buffer), "%d", value);
            return operator<<(buffer);
        }
    }

    inline StringOutputStream& StringOutputStream::operator<<(const Guid& guid)
    {
        return operator<<(guid.toString());
    }

    inline StringOutputStream& StringOutputStream::operator<<(const ResourceContentHash& value)
    {
        return *this << StringUtils::HexFromResourceContentHash(value);
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
    void StringOutputStream::setDecimalDigits(UInt32 digits)
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
        const UInt writeIdx = m_buffer.getLength();
        m_buffer.resize(m_buffer.getLength() + size);
        ramses_capu::Memory::Copy(&m_buffer[writeIdx], data, size);
        return *this;
    }

    inline
    String StringOutputStream::release()
    {
        String tmp;
        tmp.swap(m_buffer);
        return tmp;
    }
}

#endif
