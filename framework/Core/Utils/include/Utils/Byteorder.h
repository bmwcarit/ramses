//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BYTEORDER_H
#define RAMSES_BYTEORDER_H

#include "ramses-capu/os/Byteorder.h"

namespace ramses_internal
{
    class Byteorder
    {
    public:
        static UInt32 NtoHl(UInt32 netlong)
        {
            return ramses_capu::Byteorder::NtoHl(netlong);
        }

        static UInt16 NtoHs(UInt16 netshort)
        {
            return ramses_capu::Byteorder::NtoHs(netshort);
        }

        static UInt32 HtoNl(UInt32 hostlong)
        {
            return ramses_capu::Byteorder::HtoNl(hostlong);
        }

        static UInt16 HtoNs(UInt16 hostshort)
        {
            return ramses_capu::Byteorder::HtoNs(hostshort);
        }
    };
}

#endif // RAMSES_BYTEORDER_H
