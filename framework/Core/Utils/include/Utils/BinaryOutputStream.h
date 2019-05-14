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
#include "Collections/IOutputStream.h"

namespace ramses_internal
{
    class BinaryOutputStream: public IOutputStream
    {
    public:
        explicit BinaryOutputStream(UInt32 startSize = 16);

        IOutputStream& write(const void* data, const UInt32 size) override;

        const Char* getData() const;
        UInt32 getSize() const;
        UInt32 getCapacity() const;

        std::vector<char> release();

    private:
        std::vector<char> m_buffer;
    };

    inline
    BinaryOutputStream::BinaryOutputStream(UInt32 startSize)
        : m_buffer()
    {
        m_buffer.reserve(startSize);
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

    inline std::vector<char> BinaryOutputStream::release()
    {
        std::vector<char> result;
        result.swap(m_buffer);
        return result;
    }
}

#endif
