//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYINPUTSTREAM_H
#define RAMSES_BINARYINPUTSTREAM_H

#include "Collections/IInputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformError.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/String.h"
#include "Collections/Guid.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class BinaryInputStream: public IInputStream
    {
    public:
        explicit BinaryInputStream(const Char* input);
        explicit BinaryInputStream(const UChar* input);

        IInputStream& operator>>(Int32& value) override;
        IInputStream& operator>>(UInt32& value) override;
        IInputStream& operator>>(Int64& value) override;
        IInputStream& operator>>(UInt64& value) override;
        IInputStream& operator>>(String& value) override;
        IInputStream& operator>>(Bool& value) override;
        IInputStream& operator>>(Float& value) override;
        IInputStream& operator>>(UInt16& value) override;
        IInputStream& operator>>(Guid& value) override;
        IInputStream& operator>>(Matrix44f& value) override;
        IInputStream& operator>>(ResourceContentHash& value) override;
        IInputStream& read(Char* buffer, UInt32 size) override;
        IInputStream& read(UChar* buffer, UInt32 size);

        virtual EStatus getState() const  override;

    private:
        const char* m_current;
    };

    inline BinaryInputStream::BinaryInputStream(const Char* input)
        : m_current(input)
    {
    }

    inline BinaryInputStream::BinaryInputStream(const UChar* input)
        : BinaryInputStream(reinterpret_cast<const Char*>(input))
    {
    }

    inline IInputStream& BinaryInputStream::operator>>(Int32& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(Int32));
    }

    inline IInputStream& BinaryInputStream::operator>>(UInt32& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(UInt32));
    }

    inline IInputStream& BinaryInputStream::operator>>(Int64& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(Int64));
    }

    inline IInputStream& BinaryInputStream::operator>>(UInt64& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(UInt64));
    }

    inline IInputStream& BinaryInputStream::operator>>(String& value)
    {
        uint32_t length = 0;
        operator>>(length); // first read the length of the string
        String retValue;
        if (length > 0)
        {
            retValue.resize(length);
            read(retValue.data(), length);
        }
        value.swap(retValue);
        return *this;
    }

    inline IInputStream& BinaryInputStream::operator>>(Bool& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(Bool));
    }

    inline IInputStream& BinaryInputStream::operator>>(Float& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(Float));
    }

    inline IInputStream& BinaryInputStream::operator>>(UInt16& value)
    {
        return read(reinterpret_cast<char*>(&value), sizeof(UInt16));
    }

    inline IInputStream& BinaryInputStream::operator>>(Guid& value)
    {
        generic_uuid_t fromStream;
        read(reinterpret_cast<char*>(&fromStream), sizeof(generic_uuid_t));
        value = Guid(fromStream);
        return *this;
    }

    inline IInputStream& BinaryInputStream::operator>>(ResourceContentHash& value)
    {
        return *this >> value.lowPart >> value.highPart;
    }

    inline IInputStream& BinaryInputStream::operator>>(Matrix44f& value)
    {
        read(reinterpret_cast<Char*>(value.getRawData()), sizeof(Float) * 16);
        return *this;
    }

    inline IInputStream& BinaryInputStream::read(Char* buffer, UInt32 size)
    {
        PlatformMemory::Copy(buffer, m_current, size);
        m_current += size;
        return *this;
    }

    inline IInputStream& BinaryInputStream::read(UChar* data, UInt32 size)
    {
        return read(reinterpret_cast<Char*>(data), size);
    }

    inline EStatus BinaryInputStream::getState() const
    {
        return EStatus_RAMSES_OK;
    }
}

#endif
