//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VECTORBINARYOUTPUTSTREAM_H
#define RAMSES_VECTORBINARYOUTPUTSTREAM_H

#include "Collections/IOutputStream.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "absl/types/span.h"
#include <vector>

namespace ramses_internal
{
    class VectorBinaryOutputStream: public IOutputStream
    {
    public:
        explicit VectorBinaryOutputStream(std::vector<Byte>& vecRef);

        [[nodiscard]] absl::Span<const Byte> asSpan() const;

        IOutputStream& write(const void* data, size_t size) override;
    private:
        std::vector<Byte>& m_vecRef;
        const size_t m_startSize;
    };

    inline VectorBinaryOutputStream::VectorBinaryOutputStream(std::vector<Byte>& vecRef)
        : m_vecRef(vecRef)
        , m_startSize(m_vecRef.size())
    {
    }

    inline IOutputStream& VectorBinaryOutputStream::write(const void* data, size_t size)
    {
        if (size > 0)
        {
            const Byte* dataByte = static_cast<const Byte*>(data);
            m_vecRef.insert(m_vecRef.end(), dataByte, dataByte + size);
        }
        return *this;
    }

    inline absl::Span<const Byte> VectorBinaryOutputStream::asSpan() const
    {
        if (m_vecRef.empty())
            return {};
        return {m_vecRef.data() + m_startSize, m_vecRef.size() - m_startSize};
    }
}

#endif
