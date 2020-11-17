//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EROTATIONCONVENTION_H
#define RAMSES_EROTATIONCONVENTION_H

namespace ramses
{
    /**
     * Specifies the rotation convention used in calculation of transforms in a right-handed coordinate system
    */
    enum class ERotationConvention
    {
        XYZ,        ///< Euler rotation convention XYZ
        XZY,        ///< Euler rotation convention XZY
        YXZ,        ///< Euler rotation convention YXZ
        YZX,        ///< Euler rotation convention YZX
        ZXY,        ///< Euler rotation convention ZXY
        ZYX,        ///< Euler rotation convention ZYX
        XYX,        ///< Euler rotation convention XYX
        XZX,        ///< Euler rotation convention XZX
        YXY,        ///< Euler rotation convention YXY
        YZY,        ///< Euler rotation convention YZY
        ZXZ,        ///< Euler rotation convention ZXZ
        ZYZ,        ///< Euler rotation convention ZYZ
    };
}

#endif
