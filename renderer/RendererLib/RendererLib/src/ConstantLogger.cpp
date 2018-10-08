//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/ConstantLogger.h"

namespace ramses_internal
{
    template <typename DATATYPE>
    void ConstantLogger::LogValue(DataFieldHandle field, const DATATYPE& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load " << value << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Float>(DataFieldHandle field, const Float& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load float(" << value << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector2>(DataFieldHandle field, const Vector2& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec2f(" << value.x << "," << value.y << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector3>(DataFieldHandle field, const Vector3& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec3f(" << value.x << "," << value.y << "," << value.z << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector4>(DataFieldHandle field, const Vector4& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec4f(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Int32>(DataFieldHandle field, const Int32& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load int(" << value << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector2i>(DataFieldHandle field, const Vector2i& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec2i(" << value.x << "," << value.y << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector3i>(DataFieldHandle field, const Vector3i& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec3i(" << value.x << "," << value.y << "," << value.z << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Vector4i>(DataFieldHandle field, const Vector4i& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec4i(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Matrix22f>(DataFieldHandle field, const Matrix22f& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat22f(" << value.m11 << "," << value.m12 << ";" << RendererLogContext::NewLine;
        context << "            " << value.m21 << "," << value.m22 << ";" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Matrix33f>(DataFieldHandle field, const Matrix33f& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat33f(" << value.m(0, 0) << "," << value.m(0, 1) << "," << value.m(0, 2) << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value.m(1, 0) << "," << value.m(1, 1) << "," << value.m(1, 2) << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value.m(2, 0) << "," << value.m(2, 1) << "," << value.m(2, 2) << "," << ";" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<Matrix44f>(DataFieldHandle field, const Matrix44f& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat44f(" << value.m(0, 0) << "," << value.m(0, 1) << "," << value.m(0, 2) << "," << value.m(0, 3) << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value.m(1, 0) << "," << value.m(1, 1) << "," << value.m(1, 2) << "," << value.m(1, 3) << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value.m(2, 0) << "," << value.m(2, 1) << "," << value.m(2, 2) << "," << value.m(2, 3) << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value.m(3, 0) << "," << value.m(3, 1) << "," << value.m(3, 2) << "," << value.m(3, 3) << "," << ";" << RendererLogContext::NewLine;
    }
}
