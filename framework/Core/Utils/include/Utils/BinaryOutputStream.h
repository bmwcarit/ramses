//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYOUTPUTSTREAM_H
#define RAMSES_BINARYOUTPUTSTREAM_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Collections/IOutputStream.h"
#include "Collections/Guid.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class BinaryOutputStream: public IOutputStream
    {
    public:
        explicit BinaryOutputStream(UInt32 startSize = 16);

        IOutputStream& operator<<(const Float value) override;
        IOutputStream& operator<<(const Int32 value) override;
        IOutputStream& operator<<(const UInt32 value) override;
        IOutputStream& operator<<(const Int64 value) override;
        IOutputStream& operator<<(const UInt64 value) override;
        IOutputStream& operator<<(const String& value) override;
        IOutputStream& operator<<(const Bool value) override;
        IOutputStream& operator<<(const Char* value) override;
        IOutputStream& operator<<(const Guid& value) override;
        IOutputStream& operator<<(const UInt16 value) override;
        IOutputStream& operator<<(const Matrix44f& value) override;
        IOutputStream& operator<<(const ResourceContentHash& value) override;
        IOutputStream& write(const void* data, const UInt32 size) override;

        EStatus flush() override;

        const Char* getData() const;
        UInt32 getSize() const;
        UInt32 getCapacity() const;

    private:
        Vector<char> m_buffer;
    };

    inline
    BinaryOutputStream::BinaryOutputStream(UInt32 startSize)
        : m_buffer()
    {
        m_buffer.reserve(startSize);
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Float value)
    {
        return write(&value, sizeof(Float));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Int32 value)
    {
        return write(&value, sizeof(Int32));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const UInt32 value)
    {
        return write(&value, sizeof(UInt32));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Int64 value)
    {
        return write(&value, sizeof(Int64));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const UInt64 value)
    {
        return write(&value, sizeof(UInt64));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const String& value)
    {
        operator<<(static_cast<uint32_t>(value.getLength())); // first write length of string
        return write(value.c_str(), static_cast<uint32_t>(value.getLength()));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Bool value)
    {
        return write(&value, sizeof(Bool));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Char* value)
    {
        const uint32_t len = static_cast<uint32_t>(ramses_capu::StringUtils::Strlen(value));
        operator<<(len); // first write length of string
        return write(value, len);
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const UInt16 value)
    {
        return write(&value, sizeof(UInt16));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Guid& value)
    {
        return write(&value.getGuidData(), sizeof(generic_uuid_t));
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const ResourceContentHash& value)
    {
        *this << value.lowPart << value.highPart;
        return *this;
    }

    inline IOutputStream& BinaryOutputStream::operator<<(const Matrix44f& value)
    {
        return write(value.getRawData(), sizeof(Float) * 16);
    }

    inline IOutputStream& BinaryOutputStream::write(const void* data, const UInt32 size)
    {
        const char* dataCharptr = static_cast<const char*>(data);
        m_buffer.insert(m_buffer.end(), dataCharptr, dataCharptr+size);
        return *this;
    }

    inline const Char* BinaryOutputStream::getData() const
    {
        return m_buffer.data();
    }

    inline UInt32 BinaryOutputStream::getSize() const
    {
        return static_cast<uint32_t>(m_buffer.size());
    }

    inline UInt32 BinaryOutputStream::getCapacity() const
    {
        return static_cast<uint32_t>(m_buffer.capacity());
    }

    inline EStatus BinaryOutputStream::flush()
    {
        return EStatus_RAMSES_OK;
    }
}

#endif
