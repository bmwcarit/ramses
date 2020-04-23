//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VARIANT_H
#define RAMSES_VARIANT_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include "Math3d/Matrix22f.h"
#include "Math3d/Matrix33f.h"
#include "Math3d/Matrix44f.h"
#include "Utils/DataTypeUtils.h"
#include <cassert>

namespace ramses_internal
{
    class Variant
    {
    public:
        Variant();

        template <typename T>
        T getValue() const;

        template <typename T>
        void setValue(const T& data);

    private:
        union
        {
            Float m_floatData[16];
            Int32 m_int32Data[4];
            bool m_boolData;
        } m_data;

        EDataTypeID m_dataTypeID;
    };

    inline Variant::Variant()
        : m_dataTypeID(EDataTypeID_Invalid)
    {
        PlatformMemory::Set(&m_data, 0, sizeof(m_data));
    }

    template <typename T>
    inline T Variant::getValue() const
    {
        assert(false);
        return T();
    }
    template <>
    inline bool Variant::getValue<bool>() const
    {
        assert(m_dataTypeID == EDataTypeID_Boolean);
        return m_data.m_boolData;
    }
    template <>
    inline Int32 Variant::getValue<Int32>() const
    {
        assert(m_dataTypeID == EDataTypeID_Int32);
        return m_data.m_int32Data[0];
    }
    template <>
    inline Float Variant::getValue<Float>() const
    {
        assert(m_dataTypeID == EDataTypeID_Float);
        return m_data.m_floatData[0];
    }
    template <>
    inline Vector2 Variant::getValue<Vector2>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector2f);
        return Vector2(m_data.m_floatData[0], m_data.m_floatData[1]);
    }
    template <>
    inline Vector3 Variant::getValue<Vector3>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector3f);
        return Vector3(m_data.m_floatData[0], m_data.m_floatData[1], m_data.m_floatData[2]);
    }
    template <>
    inline Vector4 Variant::getValue<Vector4>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector4f);
        return Vector4(m_data.m_floatData[0], m_data.m_floatData[1], m_data.m_floatData[2], m_data.m_floatData[3]);
    }
    template <>
    inline Vector2i Variant::getValue<Vector2i>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector2i);
        return Vector2i(m_data.m_int32Data[0], m_data.m_int32Data[1]);
    }
    template <>
    inline Vector3i Variant::getValue<Vector3i>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector3i);
        return Vector3i(m_data.m_int32Data[0], m_data.m_int32Data[1], m_data.m_int32Data[2]);
    }
    template <>
    inline Vector4i Variant::getValue<Vector4i>() const
    {
        assert(m_dataTypeID == EDataTypeID_Vector4i);
        return Vector4i(m_data.m_int32Data[0], m_data.m_int32Data[1], m_data.m_int32Data[2], m_data.m_int32Data[3]);
    }
    template <>
    inline Matrix22f Variant::getValue<Matrix22f>() const
    {
        assert(m_dataTypeID == EDataTypeID_Matrix22f);
        Matrix22f mat;
        PlatformMemory::Copy(&mat.m11, &m_data.m_floatData, 4u * sizeof(Float));
        return mat;
    }
    template <>
    inline Matrix33f Variant::getValue<Matrix33f>() const
    {
        assert(m_dataTypeID == EDataTypeID_Matrix33f);
        Matrix33f mat;
        PlatformMemory::Copy(&mat.m11, &m_data.m_floatData, 9u * sizeof(Float));
        return mat;
    }
    template <>
    inline Matrix44f Variant::getValue<Matrix44f>() const
    {
        assert(m_dataTypeID == EDataTypeID_Matrix44f);
        Matrix44f mat;
        PlatformMemory::Copy(&mat.m11, &m_data.m_floatData, 16u * sizeof(Float));
        return mat;
    }

    template <typename T>
    inline void Variant::setValue(const T& data)
    {
        UNUSED(data);
        m_dataTypeID = EDataTypeID_Invalid;
        assert(false);
    }
    template <>
    inline void Variant::setValue(const bool& data)
    {
        m_data.m_boolData = data;
        m_dataTypeID = EDataTypeID_Boolean;
    }
    template <>
    inline void Variant::setValue(const Int32& data)
    {
        m_data.m_int32Data[0] = data;
        m_dataTypeID = EDataTypeID_Int32;
    }
    template <>
    inline void Variant::setValue(const Float& data)
    {
        m_data.m_floatData[0] = data;
        m_dataTypeID = EDataTypeID_Float;
    }
    template <>
    inline void Variant::setValue(const Vector2& data)
    {
        PlatformMemory::Copy(m_data.m_floatData, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector2f;
    }
    template <>
    inline void Variant::setValue(const Vector3& data)
    {
        PlatformMemory::Copy(m_data.m_floatData, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector3f;
    }
    template <>
    inline void Variant::setValue(const Vector4& data)
    {
        PlatformMemory::Copy(m_data.m_floatData, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector4f;
    }
    template <>
    inline void Variant::setValue(const Vector2i& data)
    {
        PlatformMemory::Copy(m_data.m_int32Data, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector2i;
    }
    template <>
    inline void Variant::setValue(const Vector3i& data)
    {
        PlatformMemory::Copy(m_data.m_int32Data, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector3i;
    }
    template <>
    inline void Variant::setValue(const Vector4i& data)
    {
        PlatformMemory::Copy(m_data.m_int32Data, data.data, sizeof(data.data));
        m_dataTypeID = EDataTypeID_Vector4i;
    }
    template <>
    inline void Variant::setValue(const Matrix22f& data)
    {
        PlatformMemory::Copy(&m_data.m_floatData, &data.m11, 4u * sizeof(Float));
        m_dataTypeID = EDataTypeID_Matrix22f;
    }
    template <>
    inline void Variant::setValue(const Matrix33f& data)
    {
        PlatformMemory::Copy(&m_data.m_floatData, &data.m11, 9u * sizeof(Float));
        m_dataTypeID = EDataTypeID_Matrix33f;
    }
    template <>
    inline void Variant::setValue(const Matrix44f& data)
    {
        PlatformMemory::Copy(&m_data.m_floatData, &data.m11, 16u * sizeof(Float));
        m_dataTypeID = EDataTypeID_Matrix44f;
    }
}

#endif
