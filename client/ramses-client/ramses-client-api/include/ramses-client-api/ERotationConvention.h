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
     * Specifies the rotation convention used in calculation of transforms in a right-handed coordinate system. The
     * order of the letters in each enum value represents the position of the corresponding rotation matrix
     * in the multiplication chain. For example XYZ means the rotation matrix will be of the shape Rx * Ry * Rz * v
     * where v is a vector or another matrix and R are right-handed rotation matrices which rotate around an
     * axis specified by the rotation order. Check the specific enum documentation for the exact rotation order
     * in terms of 'which rotation is applied first'.
    */
    enum class ERotationConvention
    {
        XYZ,        ///< rotates around Z then Y then X axis
        XZY,        ///< rotates around Y then Z then X axis
        YXZ,        ///< rotates around Z then X then Y axis
        YZX,        ///< rotates around X then Z then Y axis
        ZXY,        ///< rotates around Y then X then Z axis
        ZYX,        ///< rotates around X then Y then Z axis
        XYX,        ///< rotates around X then Y then X axis
        XZX,        ///< rotates around X then Z then X axis
        YXY,        ///< rotates around Y then X then Y axis
        YZY,        ///< rotates around Y then Z then Y axis
        ZXZ,        ///< rotates around Z then X then Z axis
        ZYZ,        ///< rotates around Z then Y then Z axis
    };
}

#endif
