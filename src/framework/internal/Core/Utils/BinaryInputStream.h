//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IInputStream.h"

namespace ramses::internal
{
    class BinaryInputStream: public IInputStream
    {
    public:
        explicit BinaryInputStream(const std::byte* input);

        IInputStream& read(void* buffer, size_t size) override;

        [[nodiscard]] EStatus getState() const  override;

        [[nodiscard]] const std::byte* readPosition() const;
        [[nodiscard]] size_t getCurrentReadBytes() const;
        void skip(int64_t offset);

        EStatus seek(int64_t numberOfBytesToSeek, Seek origin) override;
        EStatus getPos(size_t& position) const override;

    private:
        const std::byte* m_current;
        const std::byte* m_start;
    };

    inline BinaryInputStream::BinaryInputStream(const std::byte* input)
        : m_current(input)
        , m_start(input)
    {
    }

    inline IInputStream& BinaryInputStream::read(void* buffer, size_t size)
    {
        if (size != 0u)
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

    inline const std::byte* BinaryInputStream::readPosition() const
    {
        return m_current;
    }

    inline void BinaryInputStream::skip(int64_t offset)
    {
        m_current += offset;
    }

    inline EStatus BinaryInputStream::seek(int64_t numberOfBytesToSeek, Seek origin)
    {
        if (origin == Seek::FromBeginning)
        {
            m_current = m_start + numberOfBytesToSeek;
        }
        else if (origin == Seek::Relative)
        {
            m_current += numberOfBytesToSeek;
        }
        return EStatus::Ok;
    }

    inline EStatus BinaryInputStream::getPos(size_t& position) const
    {
        position = getCurrentReadBytes();
        return EStatus::Ok;
    }
}
