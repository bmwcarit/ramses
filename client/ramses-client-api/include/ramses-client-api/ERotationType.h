//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses
{
    /**
    * @ingroup CoreAPI
     * Specifies the rotation convention used in calculation of transforms in a right-handed coordinate system. The
     * order of the letters in each enum value represents the order in extrinsic notation. Extrinsic convention means
     * a rotation around any ordered set of axes ABC is done in the specified order around the original world axes which
     * are fixed. So the object gets rotated around world original axis A, then the result is rotated around world original
     * axis B, then the result of both is rotated around world original axis C.
     * This is opposite to intrinsic notation, where each axis of rotation gets affected by the rotations applied to preceding
     * axes in the rotation order, i.e., the rotation axes get rotated with objects' rotation around every axis, e.g., rotation
     * around axes ABC would result in rotation around world original axis A, then rotation around axis B from the object's
     * perspective according to its orientation from the applied rotation around axis A, then rotation around axis C from the object's perspective
     * according to its orientation as result of applied rotations axes around A and B.
     *
     * Extrinsic rotation conventions also mean that the order of axes specified is the inverse order of applying
     * rotation matrix multiplication. For example Euler_ZYX means the rotation matrix will be of the shape Rx * Ry * Rz * v
     * where v is a vector or another matrix and R are right-handed rotation matrices which rotate around an
     * axis specified by the rotation order. Check the specific enum documentation for the exact rotation order
     * in terms of 'which rotation is applied first'.
    */
    enum class ERotationType
    {
        Euler_XYZ,        ///< rotates around X then Y then Z axis in world coordinate system
        Euler_XZY,        ///< rotates around X then Z then Y axis in world coordinate system
        Euler_YXZ,        ///< rotates around Y then X then Z axis in world coordinate system
        Euler_YZX,        ///< rotates around Y then Z then X axis in world coordinate system
        Euler_ZXY,        ///< rotates around Z then X then Y axis in world coordinate system
        Euler_ZYX,        ///< rotates around Z then Y then X axis in world coordinate system
        Euler_XYX,        ///< rotates around X then Y then X axis in world coordinate system
        Euler_XZX,        ///< rotates around X then Z then X axis in world coordinate system
        Euler_YXY,        ///< rotates around Y then X then Y axis in world coordinate system
        Euler_YZY,        ///< rotates around Y then Z then Y axis in world coordinate system
        Euler_ZXZ,        ///< rotates around Z then X then Z axis in world coordinate system
        Euler_ZYZ,        ///< rotates around Z then Y then Z axis in world coordinate system
        Quaternion,       ///< rotation is defined by a normalized quaternion
    };
}
