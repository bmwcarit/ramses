//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "absl/types/span.h"

#include <cstdint>
#include <vector>

namespace ramses::internal
{
    class VectorBinaryOutputStream: public IOutputStream
    {
    public:
        explicit VectorBinaryOutputStream(std::vector<std::byte>& vecRef);

        [[nodiscard]] absl::Span<const std::byte> asSpan() const;

        IOutputStream& write(const void* data, size_t size) override;
        EStatus getPos(size_t& position) const override;
    private:
        std::vector<std::byte>& m_vecRef;
        const size_t m_startSize;
    };

    inline VectorBinaryOutputStream::VectorBinaryOutputStream(std::vector<std::byte>& vecRef)
        : m_vecRef(vecRef)
        , m_startSize(m_vecRef.size())
    {
    }

    inline IOutputStream& VectorBinaryOutputStream::write(const void* data, size_t size)
    {
        if (size > 0)
        {
            const auto* dataByte = static_cast<const std::byte*>(data);
            m_vecRef.insert(m_vecRef.end(), dataByte, dataByte + size);
        }
        return *this;
    }

    inline EStatus VectorBinaryOutputStream::getPos(size_t& position) const
    {
        position = m_vecRef.size() - m_startSize;
        return EStatus::Ok;
    }

    inline absl::Span<const std::byte> VectorBinaryOutputStream::asSpan() const
    {
        if (m_vecRef.empty())
            return {};
        return {m_vecRef.data() + m_startSize, m_vecRef.size() - m_startSize};
    }
}
