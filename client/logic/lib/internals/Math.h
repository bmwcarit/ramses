//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <array>

namespace rlogic::internal::math
{
    class Vector4
    {
    public:
        Vector4() = default;

        Vector4(float vx, float vy, float vz, float vw)
            : x{ vx }
            , y{ vy }
            , z{ vz }
            , w{ vw }
        {
        }

        Vector4 operator*(const Vector4& vec) const
        {
            return Vector4(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
        }

        Vector4 operator*(float scalar) const
        {
            return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        Vector4 operator/(float scalar) const
        {
            return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
        }

        Vector4 operator+(float scalar) const
        {
            return Vector4(x + scalar, y + scalar, z + scalar, w + scalar);
        }

        Vector4 operator-(float scalar) const
        {
            return Vector4(x - scalar, y - scalar, z - scalar, w - scalar);
        }

        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        float w = 0.f;
    };

    class Matrix44f
    {
    public:
        float m11 = 1.f;
        float m21 = 0.f;
        float m31 = 0.f;
        float m41 = 0.f;
        float m12 = 0.f;
        float m22 = 1.f;
        float m32 = 0.f;
        float m42 = 0.f;
        float m13 = 0.f;
        float m23 = 0.f;
        float m33 = 1.f;
        float m43 = 0.f;
        float m14 = 0.f;
        float m24 = 0.f;
        float m34 = 0.f;
        float m44 = 1.f;

        Matrix44f() = default;

        Matrix44f(float _m11, float _m12, float _m13, float _m14
            , float _m21, float _m22, float _m23, float _m24
            , float _m31, float _m32, float _m33, float _m34
            , float _m41, float _m42, float _m43, float _m44)
            : m11(_m11), m21(_m21), m31(_m31), m41(_m41)
            , m12(_m12), m22(_m22), m32(_m32), m42(_m42)
            , m13(_m13), m23(_m23), m33(_m33), m43(_m43)
            , m14(_m14), m24(_m24), m34(_m34), m44(_m44)
        {
        }

        // NOLINTNEXTLINE(modernize-avoid-c-arrays) Ramses uses C array in matrix getters
        explicit Matrix44f(const float(&data)[16])
        {
            m11 = data[0];
            m21 = data[1];
            m31 = data[2];
            m41 = data[3];
            m12 = data[4];
            m22 = data[5];
            m32 = data[6];
            m42 = data[7];
            m13 = data[8];
            m23 = data[9];
            m33 = data[10];
            m43 = data[11];
            m14 = data[12];
            m24 = data[13];
            m34 = data[14];
            m44 = data[15];
        }

        explicit Matrix44f(const std::array<float, 16>& data)
        {
            m11 = data[0];
            m21 = data[1];
            m31 = data[2];
            m41 = data[3];
            m12 = data[4];
            m22 = data[5];
            m32 = data[6];
            m42 = data[7];
            m13 = data[8];
            m23 = data[9];
            m33 = data[10];
            m43 = data[11];
            m14 = data[12];
            m24 = data[13];
            m34 = data[14];
            m44 = data[15];
        }

        [[nodiscard]] std::array<float, 16> toStdArray() const
        {
            return std::array<float, 16>{
                m11, m21, m31, m41,
                m12, m22, m32, m42,
                m13, m23, m33, m43,
                m14, m24, m34, m44
            };
        }

        Matrix44f operator*(const Matrix44f& mat) const
        {
            return Matrix44f(m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31 + m14 * mat.m41, m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32 + m14 * mat.m42,
                m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33 + m14 * mat.m43, m11 * mat.m14 + m12 * mat.m24 + m13 * mat.m34 + m14 * mat.m44
                , m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31 + m24 * mat.m41, m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32 + m24 * mat.m42,
                m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33 + m24 * mat.m43, m21 * mat.m14 + m22 * mat.m24 + m23 * mat.m34 + m24 * mat.m44
                , m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31 + m34 * mat.m41, m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32 + m34 * mat.m42,
                m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33 + m34 * mat.m43, m31 * mat.m14 + m32 * mat.m24 + m33 * mat.m34 + m34 * mat.m44
                , m41 * mat.m11 + m42 * mat.m21 + m43 * mat.m31 + m44 * mat.m41, m41 * mat.m12 + m42 * mat.m22 + m43 * mat.m32 + m44 * mat.m42,
                m41 * mat.m13 + m42 * mat.m23 + m43 * mat.m33 + m44 * mat.m43, m41 * mat.m14 + m42 * mat.m24 + m43 * mat.m34 + m44 * mat.m44);
        }

        Vector4 operator*(const Vector4& vec) const
        {
            return Vector4(m11 * vec.x + m12 * vec.y + m13 * vec.z + m14 * vec.w
                , m21 * vec.x + m22 * vec.y + m23 * vec.z + m24 * vec.w
                , m31 * vec.x + m32 * vec.y + m33 * vec.z + m34 * vec.w
                , m41 * vec.x + m42 * vec.y + m43 * vec.z + m44 * vec.w);
        }
    };
}
