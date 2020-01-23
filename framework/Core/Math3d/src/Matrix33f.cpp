//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Math3d/Matrix33f.h>
#include <Math3d/Matrix44f.h>

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

    Matrix33f Matrix33f::RotationEulerXYZ(const Vector3& rotationXYZ)
    {
        // Get the rotation angles in radians
        const float rotX = PlatformMath::Deg2Rad(rotationXYZ.x);
        const float rotY = PlatformMath::Deg2Rad(rotationXYZ.y);
        const float rotZ = PlatformMath::Deg2Rad(rotationXYZ.z);

        // Save some sin and cos values for reuse in the computations
        const float sinX = sin(rotX);
        const float cosX = cos(rotX);
        const float sinY = sin(rotY);
        const float cosY = cos(rotY);
        const float sinZ = sin(rotZ);
        const float cosZ = cos(rotZ);

        // Creation form below utilizes sign change in comparison to regular XYZ construction.
        // This change is tolerated back when angles are extracted back from the matrix in toRotationEulerXYZ() method

        return Matrix33f(
            cosY * cosZ,                      cosY * sinZ,                      -sinY      ,
            sinX * sinY * cosZ - cosX * sinZ, cosX * cosZ + sinX * sinY * sinZ, sinX * cosY,
            sinX * sinZ + cosX * sinY * cosZ, cosX * sinY * sinZ - sinX * cosZ, cosX * cosY);
    }

    bool Matrix33f::toRotationEulerXYZ(Vector3& rotationXYZ) const
    {
        return toRotationEulerXYZ(rotationXYZ.x, rotationXYZ.y, rotationXYZ.z);
    }

    bool Matrix33f::toRotationEulerXYZ(Float& x, Float& y, Float& z) const
    {
        // Creation from XYZ Euler angles utilizes sign change in comparison to regular XYZ construction.
        // This change is tolerated back in here by inverting the angles at the end of the function

        // For details about factor calculation, see https://www.geometrictools.com/Documentation/EulerAngles.pdf
        bool singular = true;

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

        // invert and convert radiant values to degrees
        x = -PlatformMath::Rad2Deg(x);
        y = -PlatformMath::Rad2Deg(y);
        z = -PlatformMath::Rad2Deg(z);

        return singular;
    }

    Matrix33f Matrix33f::RotationEulerZYX(const Vector3& rotationXYZ)
    {
        // Get the rotation angles in radians
        const float rotX = PlatformMath::Deg2Rad(rotationXYZ.x);
        const float rotY = PlatformMath::Deg2Rad(rotationXYZ.y);
        const float rotZ = PlatformMath::Deg2Rad(rotationXYZ.z);

        // Save some sin and cos values for reuse in the computations
        const float sinX = sin(rotX);
        const float cosX = cos(rotX);
        const float sinY = sin(rotY);
        const float cosY = cos(rotY);
        const float sinZ = sin(rotZ);
        const float cosZ = cos(rotZ);

        // Creation form below utilizes sign change in comparison to regular ZYX construction.
        // This change is tolerated back when angles are extracted back from the matrix in toRotationEulerZYX() method

        return Matrix33f(
            cosZ * cosY,    cosZ * sinY * sinX + sinZ * cosX,   sinZ * sinX - cosZ * sinY * cosX,
            -sinZ * cosY,   cosZ * cosX - sinZ * sinY * sinX,   sinZ * sinY * cosX + cosZ * sinX,
            sinY,           -cosY * sinX,                       cosY * cosX);
    }

    Matrix33f Matrix33f::RotationEulerZYX(Float x, Float y, Float z)
    {
        return RotationEulerZYX(Vector3(x, y, z));
    }

    bool Matrix33f::toRotationEulerZYX(Vector3& rotationXYZ) const
    {
        return toRotationEulerZYX(rotationXYZ.x, rotationXYZ.y, rotationXYZ.z);
    }

    bool Matrix33f::toRotationEulerZYX(Float& x, Float& y, Float& z) const
    {
        // Creation from XYZ Euler angles utilizes sign change in comparison to regular XYZ construction.
        // This change is tolerated back in here with an embedded pre-(-1) multiplication in angle values

        bool singular = true;

        if (m31 < 1.0f)
        {
            if (m31 > -1.0f)
            {
                x = std::atan2(m32, m33);
                y = std::asin(-m31);
                z = std::atan2(m21, m11);
            }
            else
            {
                x = 0.0f;// any arbitrary angle
                y = PlatformMath::PI_f / 2.0f;
                z = -std::atan2(-m23, m22);
                // Not a unique solution.
                singular = false;
            }
        }
        else
        {
            x = 0.0f; // any arbitrary angle
            y = -PlatformMath::PI_f / 2.0f;
            z = std::atan2(-m23, m22);
            // Not a unique solution.
            singular = false;
        }

        // invert and convert radiant values to degrees
        x = -PlatformMath::Rad2Deg(x);
        y = -PlatformMath::Rad2Deg(y);
        z = -PlatformMath::Rad2Deg(z);

        return singular;
    }


    Float& Matrix33f::m(UInt32 column, UInt32 row)
    {
        assert(row < 3);
        assert(column < 3);
        return *(&m11 + (row * 3) + column);
    }

    const ramses_internal::Float& Matrix33f::m(UInt32 column, UInt32 row) const
    {
        assert(row < 3);
        assert(column < 3);
        return *(&m11 + (row * 3) + column);
    }

}
