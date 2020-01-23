//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestPngHeader.h"
#include <numeric>

namespace ramses_internal
{
    namespace TestPngHeader
    {
        std::vector<Byte> GetValidHeader()
        {
            // 33 header bytes created via "xxd -i -l33 framework/test/res/sampleImage.png"
            return {
                0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
                0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20,
                0x08, 0x02, 0x00, 0x00, 0x00, 0xfc, 0x18, 0xed, 0xa3
            };
        }

        std::vector<Byte> GetValidHeaderWithFakeData()
        {
            auto vec = GetValidHeader();
            const auto dataStart = vec.size();
            vec.resize(2000);
            std::iota(vec.begin() + dataStart, vec.end(), static_cast<unsigned char>(10));
            return vec;
        }

        std::vector<Byte> GetInvalidHeader()
        {
            auto vec = GetValidHeader();
            vec[6] = 0;
            return vec;
        }
    }
}
