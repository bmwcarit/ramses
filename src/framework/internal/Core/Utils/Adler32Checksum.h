//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>

namespace ramses::internal
{
    class Adler32Checksum
    {
    public:

        [[nodiscard]] uint32_t getResult() const
        {
            return (m_b << 16) | m_a;
        }

        void addData(const std::byte* ptr, uint32_t size)
        {
            for (uint32_t i = 0; i < size; i++)
            {
                m_a = (m_a + std::to_integer<uint32_t>(ptr[i])) % MOD_ADLER;
                m_b = (m_b + m_a) % MOD_ADLER;
            }
        }

    private:
        const int MOD_ADLER = 65521;

        uint32_t m_a = 1;
        uint32_t m_b = 0;
    };
}

