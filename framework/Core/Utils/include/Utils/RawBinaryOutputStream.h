//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAWBINARYOUTPUTSTREAM_H
#define RAMSES_RAWBINARYOUTPUTSTREAM_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/IOutputStream.h"

namespace ramses_internal
{
    class RawBinaryOutputStream: public IOutputStream
    {
    public:
        explicit RawBinaryOutputStream(UInt8* data, UInt32 size);

        IOutputStream& write(const void* data, const UInt32 size) override;

        const UInt8* getData() const;
        UInt32 getSize() const;
        UInt32 getBytesWritten() const;

    private:
        template<typename T>
        void writeBaseType(const T& value);

        UInt8* m_dataBase;
        UInt8* m_data;
        UInt32 m_size;
    };

    inline
    RawBinaryOutputStream::RawBinaryOutputStream(UInt8* data, UInt32 size)
    : m_dataBase(data)
    , m_data(data)
    , m_size(size)
    {
        assert(data != NULL && size > 0u);
    }

    template<typename T>
    inline void RawBinaryOutputStream::writeBaseType(const T& value)
    {
        assert(m_data + sizeof(T) <= m_dataBase + m_size);
        PlatformMemory::Copy(m_data, &value, sizeof(T));
        m_data += sizeof(T);
    }

    inline IOutputStream& RawBinaryOutputStream::write(const void* data, const UInt32 size)
    {
        assert(m_data + size <= m_dataBase + m_size);
        PlatformMemory::Copy(m_data, data, size);
        m_data += size;
        return *this;
    }

    inline const UInt8* RawBinaryOutputStream::getData() const
    {
        return m_dataBase;
    }

    inline UInt32 RawBinaryOutputStream::getSize() const
    {
        return m_size;
    }

    inline UInt32 RawBinaryOutputStream::getBytesWritten() const
    {
        return static_cast<UInt32>(m_data - m_dataBase);
    }
}

#endif
