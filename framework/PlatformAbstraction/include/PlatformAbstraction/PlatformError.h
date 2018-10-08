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
        EStatus_RAMSES_EINVAL          = ramses_capu::CAPU_EINVAL         ,
        EStatus_RAMSES_SOCKET_EADDR    = ramses_capu::CAPU_SOCKET_EADDR   ,
        EStatus_RAMSES_SOCKET_ESOCKET  = ramses_capu::CAPU_SOCKET_ESOCKET ,
        EStatus_RAMSES_SOCKET_ECONNECT = ramses_capu::CAPU_SOCKET_ECONNECT,
        EStatus_RAMSES_TIMEOUT         = ramses_capu::CAPU_ETIMEOUT       ,
        EStatus_RAMSES_ENO_MEMORY      = ramses_capu::CAPU_ENO_MEMORY     ,
        EStatus_RAMSES_ERROR           = ramses_capu::CAPU_ERROR          ,
        EStatus_RAMSES_NOT_EXIST       = ramses_capu::CAPU_ENOT_EXIST     ,
        EStatus_RAMSES_ERANGE          = ramses_capu::CAPU_ERANGE         ,
        EStatus_RAMSES_EOF             = ramses_capu::CAPU_EOF
    };
}

#endif
