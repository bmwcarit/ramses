//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EDATATYPE_H
#define RAMSES_EDATATYPE_H

namespace ramses
{
    /**
     * Specifies the data type used for creating data buffers
    */
    enum EDataType
    {
        EDataType_UInt16 = 0,
        EDataType_UInt32,
        EDataType_Float,
        EDataType_Vector2F,
        EDataType_Vector3F,
        EDataType_Vector4F
    };
}

#endif
