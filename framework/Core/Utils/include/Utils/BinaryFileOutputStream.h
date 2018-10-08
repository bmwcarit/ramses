//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYFILEOUTPUTSTREAM_H
#define RAMSES_BINARYFILEOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include "Utils/File.h"
#include "Collections/Guid.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    class BinaryFileOutputStream: public IOutputStream
    {
    public:
        BinaryFileOutputStream(File& file, EFileMode mode = EFileMode_WriteNewBinary);
        ~BinaryFileOutputStream();

        BinaryFileOutputStream(const BinaryFileOutputStream&) = delete;
        BinaryFileOutputStream& operator=(const BinaryFileOutputStream&) = delete;

        IOutputStream& operator<<(const Float value) override;
        IOutputStream& operator<<(const Int32 value) override;
        IOutputStream& operator<<(const UInt32 value) override;
        IOutputStream& operator<<(const Int64 value) override;
        IOutputStream& operator<<(const UInt64 value) override;
        IOutputStream& operator<<(const String& value) override;
        IOutputStream& operator<<(const Bool  value) override;
        IOutputStream& operator<<(const Char* value) override;
        IOutputStream& operator<<(const UInt16 value) override;
        IOutputStream& operator<<(const Guid& value) override;
        IOutputStream& operator<<(const Matrix44f& value) override;
        IOutputStream& operator<<(const ResourceContentHash& value) override;
        IOutputStream& write(const void* data, const UInt32 size) override;

        EStatus flush() override;

        EStatus seek(Int numberOfBytesToSeek, EFileSeekOrigin origin);
        EStatus getPos(UInt& position);

        EStatus getState() const;

    private:
        File& m_file;
        EStatus m_state;
    };

    inline
    BinaryFileOutputStream::BinaryFileOutputStream(File& file, EFileMode mode)
        : m_file(file)
        , m_state(m_file.open(mode))
    {
    }

    inline
    BinaryFileOutputStream::~BinaryFileOutputStream()
    {
        m_file.flush();
        m_file.close();
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Float value)
    {
        return write(&value, sizeof(Float));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Int32 value)
    {
        return write(&value, sizeof(Int32));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const UInt32 value)
    {
        return write(&value, sizeof(UInt32));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Int64 value)
    {
        return write(&value, sizeof(Int64));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const UInt64 value)
    {
        return write(&value, sizeof(UInt64));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const String& value)
    {
        operator<<(static_cast<uint32_t>(value.getLength())); // first write length of string
        return write(value.c_str(), static_cast<uint32_t>(value.getLength()));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Bool value)
    {
        return write(&value, sizeof(Bool));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Char* value)
    {
        const uint32_t len = static_cast<uint32_t>(ramses_capu::StringUtils::Strlen(value));
        operator<<(len); // first write length of string
        return write(value, len);
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const UInt16 value)
    {
        return write(&value, sizeof(UInt16));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Guid& value)
    {
        return write(&value.getGuidData(), sizeof(generic_uuid_t));
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const ResourceContentHash& value)
    {
        *this << value.lowPart << value.highPart;
        return *this;
    }

    inline IOutputStream& BinaryFileOutputStream::operator<<(const Matrix44f& value)
    {
        return write(value.getRawData(), sizeof(Float) * 16);
    }

    inline
    IOutputStream&
    BinaryFileOutputStream::write(const void* data, const UInt32 size)
    {
        if (EStatus_RAMSES_OK == m_state)
        {
            m_state = m_file.write(reinterpret_cast<const char*>(data), size);
        }
        return *this;
    }

    inline
    EStatus
    BinaryFileOutputStream::flush()
    {
        m_state = m_file.flush();
        return m_state;
    }

    inline
    ramses_internal::EStatus BinaryFileOutputStream::seek(Int numberOfBytesToSeek, EFileSeekOrigin origin)
    {
        m_state = m_file.seek(numberOfBytesToSeek, origin);
        return m_state;
    }

    inline
    ramses_internal::EStatus BinaryFileOutputStream::getPos(UInt& position)
    {
        m_state = m_file.getPos(position);
        return m_state;
    }

    inline
    EStatus BinaryFileOutputStream::getState() const
    {
        return m_state;
    }
}

#endif
