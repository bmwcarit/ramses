//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMERROR_H
#define RAMSES_PLATFORMERROR_H

#include <ramses-capu/Error.h>

namespace ramses_internal
{
    enum EStatus
    {
        EStatus_RAMSES_OK              = ramses_capu::CAPU_OK             ,
        EStatus_RAMSES_ERROR           = ramses_capu::CAPU_ERROR          ,
        EStatus_RAMSES_NOT_EXIST       = ramses_capu::CAPU_ENOT_EXIST     ,
        EStatus_RAMSES_EOF             = ramses_capu::CAPU_EOF
    };
}

#endif
