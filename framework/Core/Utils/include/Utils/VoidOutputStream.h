//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VOIDOUTPUTSTREAM_H
#define RAMSES_VOIDOUTPUTSTREAM_H

#include <PlatformAbstraction/PlatformTypes.h>
#include "Collections/IOutputStream.h"
#include "Collections/String.h"
#include "Collections/Guid.h"
#include "Math3d/Matrix44f.h"
#include "ramses-capu/os/StringUtils.h"

namespace ramses_internal
{
    class VoidOutputStream: public IOutputStream
    {
    public:
        explicit VoidOutputStream();

        IOutputStream& operator<<(const Int32 value) override;
        IOutputStream& operator<<(const Int64 value) override;
        IOutputStream& operator<<(const UInt32 value) override;
        IOutputStream& operator<<(const UInt64 value) override;
        IOutputStream& operator<<(const String& value) override;
        IOutputStream& operator<<(const Char* value) override;
        IOutputStream& operator<<(const Bool value) override;
        IOutputStream& operator<<(const Float value) override;
        IOutputStream& operator<<(const UInt16 value) override;
        IOutputStream& operator<<(const Guid& value) override;
        IOutputStream& operator<<(const Matrix44f& value) override;
        IOutputStream& operator<<(const ResourceContentHash& value) override;
        IOutputStream& write(const void* data, const UInt32 size) override;
        EStatus flush() override;

        UInt32 getSize() const;

    private:
        UInt32 m_size;
    };

    inline
    VoidOutputStream::VoidOutputStream()
        : m_size(0u)
    {
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Float )
    {
        m_size += sizeof(Float);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Int32 )
    {
        m_size += sizeof(Int32);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const UInt32 )
    {
        m_size += sizeof(UInt32);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Int64 )
    {
        m_size += sizeof(Int64);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const UInt64 )
    {
        m_size += sizeof(UInt64);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const String& value)
    {
        m_size += sizeof(UInt32) + static_cast<UInt32>(value.getLength());
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Bool )
    {
        m_size += sizeof(Bool);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Char* value)
    {
        m_size += sizeof(UInt32) + static_cast<UInt32>(ramses_capu::StringUtils::Strlen(value));
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const UInt16 )
    {
        m_size += sizeof(UInt16);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Guid& )
    {
        m_size += sizeof(generic_uuid_t);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const Matrix44f& )
    {
        m_size += sizeof(Float) * 16;
        return *this;
    }

    inline IOutputStream& VoidOutputStream::operator<<(const ResourceContentHash& )
    {
        m_size += sizeof(ResourceContentHash);
        return *this;
    }

    inline IOutputStream& VoidOutputStream::write(const void* , const UInt32 size)
    {
        m_size += size;
        return *this;
    }

    inline UInt32 VoidOutputStream::getSize() const
    {
        return m_size;
    }

    inline EStatus VoidOutputStream::flush()
    {
        return EStatus_RAMSES_OK;
    }
}

#endif
