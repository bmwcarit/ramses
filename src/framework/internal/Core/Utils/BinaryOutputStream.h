//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IOutputStream.h"

#include <cstdint>
#include <vector>

namespace ramses::internal
{
    class BinaryOutputStream: public IOutputStream
    {
    public:
        explicit BinaryOutputStream(size_t startSize = 16);

        IOutputStream& write(const void* data, size_t size) override;
        EStatus getPos(size_t& position) const override;

        [[nodiscard]] const std::byte* getData() const;
        [[nodiscard]] size_t getSize() const;
        [[nodiscard]] size_t getCapacity() const;

        std::vector<std::byte> release();

    private:
        std::vector<std::byte> m_buffer;
    };

    inline BinaryOutputStream::BinaryOutputStream(size_t startSize)
    {
        m_buffer.reserve(startSize);
    }

    inline IOutputStream& BinaryOutputStream::write(const void* data, size_t size)
    {
        const auto* dataCharptr = static_cast<const std::byte*>(data);
        m_buffer.insert(m_buffer.end(), dataCharptr, dataCharptr+size);
        return *this;
    }

    inline EStatus BinaryOutputStream::getPos(size_t& position) const
    {
        position = m_buffer.size();
        return EStatus::Ok;
    }

    inline const std::byte* BinaryOutputStream::getData() const
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

    inline std::vector<std::byte> BinaryOutputStream::release()
    {
        std::vector<std::byte> result;
        result.swap(m_buffer);
        return result;
    }
}
