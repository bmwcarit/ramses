//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Math3d/Matrix33f.h>
#include <Math3d/Matrix44f.h>
#include "PlatformAbstraction/PlatformMath.h"
#include <cassert>

namespace ramses_internal
{
    const Matrix33f Matrix33f::Identity(1.f, 0.f, 0.f,
                                        0.f, 1.f, 0.f,
                                        0.f, 0.f, 1.f);

    const Matrix33f Matrix33f::Empty(   0.f, 0.f, 0.f,
                                        0.f, 0.f, 0.f,
                                        0.f, 0.f, 0.f);

    Matrix33f::Matrix33f(const Matrix44f& otherMat44)
    {
        m11 = otherMat44.m11;
        m12 = otherMat44.m12;
        m13 = otherMat44.m13;
        m21 = otherMat44.m21;
        m22 = otherMat44.m22;
        m23 = otherMat44.m23;
        m31 = otherMat44.m31;
        m32 = otherMat44.m32;
        m33 = otherMat44.m33;
    }

    Matrix33f Matrix33f::Rotation(const Vector4& rotation, ERotationConvention rotationConvention)
    {
        if (rotationConvention == ERotationConvention::Quaternion)
        {
            return RotationQuaternion(rotation);
        }
        else
        {
            return RotationEuler(Vector3(rotation), rotationConvention);
        }
    }

    Matrix33f Matrix33f::RotationQuaternion(const Vector4& rotation)
    {
        const auto x  = rotation.x;
        const auto y  = rotation.y;
        const auto z  = rotation.z;
        const auto w  = rotation.w;

        const auto xx = x * x;
        const auto yy = y * y;
        const auto zz = z * z;
        const auto xy = x * y;
        const auto xz = x * z;
        const auto xw = x * w;
        const auto yz = y * z;
        const auto yw = y * w;
        const auto zw = z * w;

        Matrix33f  result;
        result.m11 = 1.f - 2.f * (yy + zz);
        result.m12 = 2.f * (xy - zw);
        result.m13 = 2.f * (xz + yw);
        result.m21 = 2.f * (xy + zw);
        result.m22 = 1.f - 2.f * (xx + zz);
        result.m23 = 2.f * (yz - xw);
        result.m31 = 2.f * (xz - yw);
        result.m32 = 2.f * (yz + xw);
        result.m33 = 1.f - 2.f * (xx + yy);
        return result;
    }

    Matrix33f Matrix33f::RotationEuler(const Vector3& rotation, ERotationConvention rotationConvention)
    {
        // Get the rotation angles in radians
        const float rotX = PlatformMath::Deg2Rad(rotation.x);
        const float rotY = PlatformMath::Deg2Rad(rotation.y);
        const float rotZ = PlatformMath::Deg2Rad(rotation.z);

        // Save some sin and cos values for reuse in the computations
        const float sx = std::sin(rotX);
        const float cx = std::cos(rotX);
        const float sy = std::sin(rotY);
        const float cy = std::cos(rotY);
        const float sz = std::sin(rotZ);
        const float cz = std::cos(rotZ);

        // Creation form below utilizes sign change in comparison to regular Euler rotation matrix construction.
        // This change is tolerated back when angles are extracted back from the matrix in toRotationEuler() method

        switch (rotationConvention)
        {
        case ERotationConvention::Euler_ZYX:
            return Matrix33f(
                cy * cz                 , -cy * sz                  , sy,
                sx * sy * cz + cx * sz  , cx * cz - sx * sy * sz    , -sx * cy,
                sx * sz - cx * sy * cz  , cx * sy * sz + sx * cz    , cx * cy
            );
        case ERotationConvention::Euler_YZX:
            return Matrix33f(
                cy * cz                 , -sz                       , cz * sy,
                sx * sy + cx * cy * sz  , cx * cz                   , -cy * sx + cx * sy * sz,
                -cx * sy  + cy * sx * sz, cz * sx                   , cx * cy + sx * sy * sz
            );
        case ERotationConvention::Euler_ZXY:
            return Matrix33f(
                cy * cz + sx * sy * sz  , cz * sx * sy - cy * sz    , cx * sy,
                cx * sz                 , cx * cz                   , -sx,
                -cz * sy + cy * sx * sz , cy * cz * sx + sy * sz    , cx * cy
            );
        case ERotationConvention::Euler_XZY:
            return Matrix33f(
                cy * cz                 , sx * sy - cx * cy * sz    , cx * sy + cy * sx * sz,
                sz                      , cx * cz                   , - cz * sx,
                -cz * sy                , cy * sx + cx * sy * sz    , cx * cy - sx * sy * sz
            );
        case ERotationConvention::Euler_YXZ:
            return Matrix33f(
                cy * cz - sx * sy * sz  , -cx * sz                  , cz * sy + cy * sx * sz,
                cz * sx * sy + cy * sz  , cx * cz                   , -cy * cz * sx + sy * sz,
                -cx * sy                , sx                        , cx * cy
            );
        case ERotationConvention::Euler_XYZ:
            return Matrix33f(
                cz * cy                 , cz * sy * sx - sz * cx    , sz * sx + cz * sy * cx,
                sz * cy                 , cz * cx + sz * sy * sx    , sz * sy * cx - cz * sx,
                -sy                     , cy * sx                   , cy * cx
            );
        case ERotationConvention::Euler_XYX: // [X0,Y,X1] = [x, y, z]
        {
            const auto& sX0 = sx;
            const auto& cX0 = cx;
            const auto& sY  = sy;
            const auto& cY  = cy;
            const auto& sX1 = sz;
            const auto& cX1 = cz;
            return Matrix33f(
                cY                      , sY * sX1                  , sY * cX1,
                sY * sX0                , cX0 * cX1 - cY * sX0 * sX1, -cY * cX1 * sX0 - cX0 * sX1,
                -sY * cX0               , cX1 * sX0 + cY * cX0 * sX1, cY *cX0 * cX1 - sX0 * sX1
            );
        }
        case ERotationConvention::Euler_XZX: // [X0,Z,X1] = [x, y, z]
        {
            const auto& sX0 = sx;
            const auto& cX0 = cx;
            const auto& sZ  = sy;
            const auto& cZ  = cy;
            const auto& sX1 = sz;
            const auto& cX1 = cz;
            return Matrix33f(
                cZ                      , -sZ * cX1                 , sZ * sX1,
                sZ * cX0                , cZ * cX0 * cX1 - sX0 * sX1, -cX1 * sX0 - cZ * cX0 * sX1,
                sZ * sX0                , cZ * cX1 * sX0 + cX0 * sX1, cX0 * cX1 - cZ * sX0 * sX1
            );
        }
        case ERotationConvention::Euler_YXY: // [Y0,X,Y1] = [x, y, z]
        {
            const auto& sY0 = sx;
            const auto& cY0 = cx;
            const auto& sX  = sy;
            const auto& cX  = cy;
            const auto& sY1 = sz;
            const auto& cY1 = cz;
            return Matrix33f(
                cY0 * cY1 - cX * sY0 * sY1  , sX * sY0              , cX * cY1 * sY0 + cY0 * sY1,
                sX * sY1                    , cX                    , -sX * cY1,
                -cY1 * sY0 - cX * cY0 * sY1 , sX * cY0              , cX * cY0 * cY1 - sY0 * sY1
            );
        }
        case ERotationConvention::Euler_YZY: // [Y0,Z,Y1] = [x, y, z]
        {
            const auto& sY0 = sx;
            const auto& cY0 = cx;
            const auto& sZ  = sy;
            const auto& cZ  = cy;
            const auto& sY1 = sz;
            const auto& cY1 = cz;
            return Matrix33f(
                cZ * cY0 * cY1 - sY0 * sY1  , -sZ * cY0             , cY1 * sY0 + cZ * cY0 * sY1,
                sZ * cY1                    , cZ                    , sZ * sY1,
                -cZ * cY1 * sY0 - cY0 * sY1 , sZ * sY0              , cY0 * cY1 - cZ * sY0 * sY1
            );
        }
        case ERotationConvention::Euler_ZXZ: // [Z0, X, Z1] = [x, y, z]
        {
            const auto& sZ0 = sx;
            const auto& cZ0 = cx;
            const auto& sX  = sy;
            const auto& cX  = cy;
            const auto& sZ1 = sz;
            const auto& cZ1 = cz;

            return Matrix33f(
                cZ0 * cZ1 - cX * sZ0 * sZ1  , -cX * cZ1 * sZ0 - cZ0 * sZ1   , sX * sZ0,
                cZ1 * sZ0 + cX * cZ0 * sZ1  , cX * cZ0 * cZ1 - sZ0 * sZ1    , -sX * cZ0,
                sX * sZ1                    , sX * cZ1                      , cX
            );
        }
        case ERotationConvention::Euler_ZYZ: // [Z0, Y, Z0] = [x, y, z]
        {
            const auto& sZ0 = sx;
            const auto& cZ0 = cx;
            const auto& sY = sy;
            const auto& cY = cy;
            const auto& sZ1 = sz;
            const auto& cZ1 = cz;

            return Matrix33f(
                cY * cZ0 * cZ1 - sZ0 * sZ1  , -cZ1 * sZ0 - cY * cZ0 * sZ1   , sY * cZ0,
                cY * cZ1 * sZ0 + cZ0 * sZ1  , cZ0 * cZ1 - cY * sZ0 * sZ1    , sY * sZ0,
                -sY * cZ1                   , sY * sZ1                      , cY
            );
        }
        default:
            assert(false);
            return {};
        }
    }

    bool Matrix33f::toRotationEuler(Vector3& rotation, ERotationConvention rotationConvention) const
    {
        // For details about factor calculation, see https://www.geometrictools.com/Documentation/EulerAngles.pdf
        bool singular = true;
        auto& x = rotation.x;
        auto& y = rotation.y;
        auto& z = rotation.z;

        switch (rotationConvention)
        {
        case ERotationConvention::Euler_ZYX:
        {
            if (m12 < 1.0f)
            {
                if (m12 > -1.0f)
                {
                    x = std::atan2(-m23, m33);
                    y = std::asin(m13);
                    z = std::atan2(-m12, m11);
                }
                else
                {
                    x = -std::atan2(m21, m22);
                    y = -PlatformMath::PI_f / 2.0f;
                    z = 0.0f; // any arbitrary angle
                    // Not a unique solution.
                    singular = false;
                }
            }
            else
            {
                x = std::atan2(m21, m22);
                y = PlatformMath::PI_f / 2.0f;
                z = 0.0f; // any arbitrary angle
                // Not a unique solution.
                singular = false;
            }

            x = PlatformMath::Rad2Deg(x);
            y = PlatformMath::Rad2Deg(y);
            z = PlatformMath::Rad2Deg(z);
        }
        break;
        default:
            //Rest are not implemented (no usecase)
            assert(false);
        }

        return singular;
    }
}
