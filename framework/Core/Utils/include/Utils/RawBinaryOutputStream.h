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
#include "Collections/IOutputStream.h"
#include <cassert>
#include <cstring>

namespace ramses_internal
{
    class RawBinaryOutputStream: public IOutputStream
    {
    public:
        explicit RawBinaryOutputStream(Byte* data, size_t size);

        IOutputStream& write(const void* data, size_t size) override;
        EStatus getPos(size_t& position) const override;

        const Byte* getData() const;
        size_t getSize() const;
        size_t getBytesWritten() const;

    private:
        Byte* m_dataBase;
        Byte* m_data;
        size_t m_size;
    };

    inline
    RawBinaryOutputStream::RawBinaryOutputStream(Byte* data, size_t size)
    : m_dataBase(data)
    , m_data(data)
    , m_size(size)
    {
        assert(data != nullptr && size > 0u);
    }

    inline IOutputStream& RawBinaryOutputStream::write(const void* data, const size_t size)
    {
        assert(m_data + size <= m_dataBase + m_size);
        if (size)
            std::memcpy(m_data, data, size);
        m_data += size;
        return *this;
    }

    inline EStatus RawBinaryOutputStream::getPos(size_t& position) const
    {
        position = m_data - m_dataBase;
        return EStatus::Ok;
    }

    inline const Byte* RawBinaryOutputStream::getData() const
    {
        return m_dataBase;
    }

    inline size_t RawBinaryOutputStream::getSize() const
    {
        return m_size;
    }

    inline size_t RawBinaryOutputStream::getBytesWritten() const
    {
        return m_data - m_dataBase;
    }
}

#endif
