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

namespace ramses_internal
{
    class BinaryInputStream: public IInputStream
    {
    public:
        explicit BinaryInputStream(const Byte* input);

        IInputStream& read(void* buffer, size_t size) override;

        [[nodiscard]] virtual EStatus getState() const  override;

        [[nodiscard]] const Byte* readPosition() const;
        [[nodiscard]] size_t getCurrentReadBytes() const;
        void skip(int64_t offset);

        EStatus seek(Int numberOfBytesToSeek, Seek origin) override;
        EStatus getPos(size_t& position) const override;

    private:
        const Byte* m_current;
        const Byte* m_start;
    };

    inline BinaryInputStream::BinaryInputStream(const Byte* input)
        : m_current(input)
        , m_start(input)
    {
    }

    inline IInputStream& BinaryInputStream::read(void* buffer, size_t size)
    {
        if (size)
            std::memcpy(buffer, m_current, size);
        m_current += size;
        return *this;
    }

    inline EStatus BinaryInputStream::getState() const
    {
        return EStatus::Ok;
    }

    inline size_t BinaryInputStream::getCurrentReadBytes() const
    {
        return m_current - m_start;
    }

    inline const Byte* BinaryInputStream::readPosition() const
    {
        return m_current;
    }

    inline void BinaryInputStream::skip(int64_t offset)
    {
        m_current += offset;
    }

    inline EStatus BinaryInputStream::seek(Int numberOfBytesToSeek, Seek origin)
    {
        if (origin == Seek::FromBeginning)
            m_current = m_start + numberOfBytesToSeek;
        else if (origin == Seek::Relative)
            m_current += numberOfBytesToSeek;
        return EStatus::Ok;
    }

    inline EStatus BinaryInputStream::getPos(size_t& position) const
    {
        position = getCurrentReadBytes();
        return EStatus::Ok;
    }
}

#endif
