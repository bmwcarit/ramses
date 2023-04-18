//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MATRIX33F_H
#define RAMSES_MATRIX33F_H

#include "SceneAPI/ERotationType.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Math3d/Vector3.h"
#include "PlatformAbstraction/FmtBase.h"
#include "Utils/AssertMovable.h"
#include "glm/mat3x3.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cassert>

namespace ramses_internal
{
    class Matrix44f;

    class Matrix33f
    {
    public:
        union
        {
            struct
            {
                // Matrix is stored column-wise (to fit GLSL convention), but elements are named row-wise
                // E.g. m1x is the first row, and m23 is the 3rd element on the 2nd row
                Float m11;
                Float m21;
                Float m31;
                Float m12;
                Float m22;
                Float m32;
                Float m13;
                Float m23;
                Float m33;
            };
            Float data[9];
        };

        static const Matrix33f Identity;
        static const Matrix33f Empty;

        constexpr Matrix33f();
        constexpr Matrix33f(const Float _m11, const Float _m12, const Float _m13,
                            const Float _m21, const Float _m22, const Float _m23,
                            const Float _m31, const Float _m32, const Float _m33);
        explicit Matrix33f(const Matrix44f& otherMat44);
        constexpr Matrix33f(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        explicit Matrix33f(const glm::mat3& other);
        constexpr Matrix33f(const Matrix33f& other) = default;
        constexpr Matrix33f(Matrix33f&& other) noexcept = default;
        constexpr Matrix33f& operator=(const Matrix33f& other) = default;
        constexpr Matrix33f& operator=(Matrix33f&& other) noexcept = default;

        constexpr void set(   const Float _m11, const Float _m12, const Float _m13
                    , const Float _m21, const Float _m22, const Float _m23
                    , const Float _m31, const Float _m32, const Float _m33);
        constexpr void set(const Float val);

        constexpr Matrix33f operator*(const Float val) const;
        constexpr Matrix33f operator/(const Float val) const;
        constexpr Matrix33f operator*(const Matrix33f& mat) const;
        constexpr void operator*=(const Matrix33f& mat);
        constexpr bool operator==(const Matrix33f& other) const;
        constexpr bool operator!=(const Matrix33f& other) const;
        Vector3 operator*(const Vector3& vec) const;

        static Matrix33f Rotation(const Vector4& rotation, ERotationType rotationType);
        bool toRotationEuler(Vector3& rotation, ERotationType rotationType) const;

        constexpr Float& m(UInt32 column, UInt32 row);
        [[nodiscard]] constexpr const Float& m(UInt32 column, UInt32 row) const;

    private:
        static Matrix33f RotationQuaternion(const Vector4& rotation);
        static Matrix33f RotationEuler(const Vector3& rotation, ERotationType rotationType);
    };

    constexpr inline
    Matrix33f::Matrix33f()
        : m11(1.f), m21(0.f), m31(0.f)
        , m12(0.f), m22(1.f), m32(0.f)
        , m13(0.f), m23(0.f), m33(1.f)
    {
    }

    constexpr inline
    Matrix33f::Matrix33f(const Float _m11, const Float _m12, const Float _m13
                        , const Float _m21, const Float _m22, const Float _m23
                        , const Float _m31, const Float _m32, const Float _m33)

        : m11(_m11), m21(_m21), m31(_m31)
        , m12(_m12), m22(_m22), m32(_m32)
        , m13(_m13), m23(_m23), m33(_m33)
    {
    }

    constexpr inline
    Matrix33f::Matrix33f(const Vector3& v1, const Vector3& v2, const Vector3& v3)
        : m11(v1[0])
        , m21(v2[0])
        , m31(v3[0])
        , m12(v1[1])
        , m22(v2[1])
        , m32(v3[1])
        , m13(v1[2])
        , m23(v2[2])
        , m33(v3[2])
    {
    }

    inline
    Matrix33f::Matrix33f(const glm::mat3& other)
    {
        auto buf = glm::value_ptr(other);
        for (size_t i = 0u; i < 9u; ++i)
        {
            data[i] = buf[i];
        }
    }


    constexpr inline
    void
    Matrix33f::set(const Float _m11, const Float _m12, const Float _m13
                , const Float _m21, const Float _m22, const Float _m23
                , const Float _m31, const Float _m32, const Float _m33)
    {
        m11 = _m11;
        m12 = _m12;
        m13 = _m13;
        m21 = _m21;
        m22 = _m22;
        m23 = _m23;
        m31 = _m31;
        m32 = _m32;
        m33 = _m33;
    }

    constexpr inline
    void
    Matrix33f::set(Float val)
    {
        m11 = m12 = m13
            = m21 = m22 = m23
            = m31 = m32 = m33
            = val;
    }

    constexpr inline
    Matrix33f
    Matrix33f::operator*(const Float val) const
    {
        return Matrix33f(   m11 * val, m12 * val, m13 * val,
                            m21 * val, m22 * val, m23 * val,
                            m31 * val, m32 * val, m33 * val);
    }

    constexpr inline
    Matrix33f
    Matrix33f::operator/(const Float val) const
    {
        return Matrix33f(   m11 / val, m12 / val, m13 / val,
                            m21 / val, m22 / val, m23 / val,
                            m31 / val, m32 / val, m33 / val);
    }

    constexpr inline
    Matrix33f
    Matrix33f::operator*(const Matrix33f& mat) const
    {
        return Matrix33f( m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31, m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32, m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33
                        , m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31, m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32, m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33
                        , m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31, m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32, m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33);
    }

    constexpr inline
    void
    Matrix33f::operator*=(const Matrix33f& mat)
    {
        *this = operator*(mat);
    }

    inline
    Vector3
    Matrix33f::operator*(const Vector3& vec) const
    {
        return Vector3(  m11 * vec.x + m12 * vec.y + m13 * vec.z
                        , m21 * vec.x + m22 * vec.y + m23 * vec.z
                        , m31 * vec.x + m32 * vec.y + m33 * vec.z);
    }

    constexpr inline
    Bool
    Matrix33f::operator==(const Matrix33f& other) const
    {
        return m11 == other.m11 &&
            m12 == other.m12 &&
            m13 == other.m13 &&
            m21 == other.m21 &&
            m22 == other.m22 &&
            m23 == other.m23 &&
            m31 == other.m31 &&
            m32 == other.m32 &&
            m33 == other.m33;
    }

    constexpr inline
    Bool
    Matrix33f::operator!=(const Matrix33f& other) const
    {
        return !operator==(other);
    }

    constexpr inline Float& Matrix33f::m(UInt32 column, UInt32 row)
    {
        assert(row < 3);
        assert(column < 3);
        return *(&m11 + (row * 3) + column);
    }

    constexpr inline const ramses_internal::Float& Matrix33f::m(UInt32 column, UInt32 row) const
    {
        assert(row < 3);
        assert(column < 3);
        return *(&m11 + (row * 3) + column);
    }

    ASSERT_MOVABLE(Matrix33f)
}

template <>
struct fmt::formatter<ramses_internal::Matrix33f> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses_internal::Matrix33f& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {}; {} {} {}; {} {} {}]",
                              m.data[0], m.data[3], m.data[6],
                              m.data[1], m.data[4], m.data[7],
                              m.data[2], m.data[5], m.data[8]);
    }
};

#endif
