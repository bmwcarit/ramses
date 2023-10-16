//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IOutputStream.h"

#include <cstdint>
#include <cassert>
#include <cstring>

namespace ramses::internal
{
    class RawBinaryOutputStream: public IOutputStream
    {
    public:
        explicit RawBinaryOutputStream(std::byte* data, size_t size);

        IOutputStream& write(const void* data, size_t size) override;
        EStatus getPos(size_t& position) const override;

        [[nodiscard]] const std::byte* getData() const;
        [[nodiscard]] size_t getSize() const;
        [[nodiscard]] size_t getBytesWritten() const;

    private:
        std::byte* m_dataBase;
        std::byte* m_data;
        size_t m_size;
    };

    inline
    RawBinaryOutputStream::RawBinaryOutputStream(std::byte* data, size_t size)
    : m_dataBase(data)
    , m_data(data)
    , m_size(size)
    {
        assert(data != nullptr && size > 0u);
    }

    inline IOutputStream& RawBinaryOutputStream::write(const void* data, const size_t size)
    {
        assert(m_data + size <= m_dataBase + m_size);
        if (size != 0u)
            std::memcpy(m_data, data, size);
        m_data += size;
        return *this;
    }

    inline EStatus RawBinaryOutputStream::getPos(size_t& position) const
    {
        position = m_data - m_dataBase;
        return EStatus::Ok;
    }

    inline const std::byte* RawBinaryOutputStream::getData() const
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
