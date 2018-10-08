//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ERESOURCECOMPRESSIONSTATUS_H
#define RAMSES_ERESOURCECOMPRESSIONSTATUS_H

namespace ramses_internal
{
    enum EResourceCompressionStatus  //enum to support multiple compression formats in the future
    {
        EResourceCompressionStatus_Uncompressed = 0,
        EResourceCompressionStatus_Compressed
    };
}

#endif // RAMSES_ERESOURCECOMPRESSIONSTATUS_H
