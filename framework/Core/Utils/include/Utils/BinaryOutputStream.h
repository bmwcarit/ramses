//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYOUTPUTSTREAM_H
#define RAMSES_BINARYOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include <vector>

namespace ramses_internal
{
    class BinaryOutputStream: public IOutputStream
    {
    public:
        explicit BinaryOutputStream(size_t startSize = 16);

        IOutputStream& write(const void* data, size_t size) override;

        const Byte* getData() const;
        size_t getSize() const;
        size_t getCapacity() const;

        std::vector<Byte> release();

    private:
        std::vector<Byte> m_buffer;
    };

    inline BinaryOutputStream::BinaryOutputStream(size_t startSize)
        : m_buffer()
    {
        m_buffer.reserve(startSize);
    }

    inline IOutputStream& BinaryOutputStream::write(const void* data, size_t size)
    {
        const Byte* dataCharptr = static_cast<const Byte*>(data);
        m_buffer.insert(m_buffer.end(), dataCharptr, dataCharptr+size);
        return *this;
    }

    inline const Byte* BinaryOutputStream::getData() const
    {
        return m_buffer.data();
    }

    inline size_t BinaryOutputStream::getSize() const
    {
        return static_cast<uint32_t>(m_buffer.size());
    }

    inline size_t BinaryOutputStream::getCapacity() const
    {
        return static_cast<uint32_t>(m_buffer.capacity());
    }

    inline std::vector<Byte> BinaryOutputStream::release()
    {
        std::vector<Byte> result;
        result.swap(m_buffer);
        return result;
    }
}

#endif
