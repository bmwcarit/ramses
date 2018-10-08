//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <Math3d/Matrix44f.h>

namespace ramses_internal
{
    const Matrix44f Matrix44f::Identity(  1.f, 0.f, 0.f, 0.f,
                                        0.f, 1.f, 0.f, 0.f,
                                        0.f, 0.f, 1.f, 0.f,
                                        0.f, 0.f, 0.f, 1.f);

    const Matrix44f Matrix44f::Empty( 0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f,
                                    0.f, 0.f, 0.f, 0.f);

    Matrix44f::Matrix44f(const Matrix33f& otherMat33)
    {
        m11 = otherMat33.m11;
        m12 = otherMat33.m12;
        m13 = otherMat33.m13;
        m14 = 0.f;
        m21 = otherMat33.m21;
        m22 = otherMat33.m22;
        m23 = otherMat33.m23;
        m24 = 0.f;
        m31 = otherMat33.m31;
        m32 = otherMat33.m32;
        m33 = otherMat33.m33;
        m34 = 0.f;
        m41 = 0.f;
        m42 = 0.f;
        m43 = 0.f;
        m44 = 1.f;
    }

    Matrix44f Matrix44f::Translation(const Vector3& translation)
    {
        return Matrix44f(
            1.0f, 0.0f, 0.0f, translation.x,
            0.0f, 1.0f, 0.0f, translation.y,
            0.0f, 0.0f, 1.0f, translation.z,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    Matrix44f Matrix44f::Translation(const Float x, const Float y, const Float z)
    {
        return Translation(Vector3(x, y, z));
    }

    Matrix44f Matrix44f::Scaling(const Vector3& scaling)
    {
        return Matrix44f(
            scaling.x,  0.0f,       0.0f,       0.0f,
            0.0f,       scaling.y,  0.0f,       0.0f,
            0.0f,       0.0f,       scaling.z,  0.0f,
            0.0f,       0.0f,       0.0f,       1.0f);
    }

    Matrix44f Matrix44f::Scaling(const Float x, const Float y, const Float z)
    {
        return Scaling(Vector3(x, y, z));
    }

    Matrix44f Matrix44f::Scaling(const Float uniScale)
    {
        return Scaling(Vector3(uniScale, uniScale, uniScale));
    }

    Vector3 Matrix44f::rotate(const Vector3& point) const
    {
        const Matrix44f rotationMatrix(Matrix33f(*this));
        const Vector4 extPoint(point.x, point.y, point.z, 1.0f);
        const Vector4 rotatedExtPoint = rotationMatrix * extPoint;

        return Vector3(rotatedExtPoint.x, rotatedExtPoint.y, rotatedExtPoint.z);
    }
}

