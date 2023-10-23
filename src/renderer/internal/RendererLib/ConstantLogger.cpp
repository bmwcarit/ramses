//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/ConstantLogger.h"

namespace ramses::internal
{
    template <typename DATATYPE>
    void ConstantLogger::LogValue(DataFieldHandle field, const DATATYPE& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load " << value << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<float>(DataFieldHandle field, const float& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load float(" << value << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::vec2>(DataFieldHandle field, const glm::vec2& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec2f(" << value.x << "," << value.y << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::vec3>(DataFieldHandle field, const glm::vec3& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec3f(" << value.x << "," << value.y << "," << value.z << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::vec4>(DataFieldHandle field, const glm::vec4& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec4f(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<int32_t>(DataFieldHandle field, const int32_t& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load int(" << value << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<bool>(DataFieldHandle field, const bool& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load bool(" << value << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::ivec2>(DataFieldHandle field, const glm::ivec2& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec2i(" << value.x << "," << value.y << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::ivec3>(DataFieldHandle field, const glm::ivec3& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec3i(" << value.x << "," << value.y << "," << value.z << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::ivec4>(DataFieldHandle field, const glm::ivec4& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load vec4i(" << value.x << "," << value.y << "," << value.z << "," << value.w << ")" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::mat2>(DataFieldHandle field, const glm::mat2& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat22f(" << value[0][0] << "," << value[1][0] << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][1] << "," << value[1][1] << ";" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::mat3>(DataFieldHandle field, const glm::mat3& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat33f(" << value[0][0] << "," << value[1][0] << "," << value[2][0] << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][1] << "," << value[1][1] << "," << value[2][1] << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][2] << "," << value[1][2] << "," << value[2][2] << "," << ";" << RendererLogContext::NewLine;
    }

    template <>
    void ConstantLogger::LogValue<glm::mat4>(DataFieldHandle field, const glm::mat4& value, RendererLogContext& context)
    {
        context << "[" << field << "] Load mat44f(" << value[0][0] << "," << value[1][0] << "," << value[2][0] << "," << value[3][0] << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][1] << "," << value[1][1] << "," << value[2][1] << "," << value[3][1] << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][2] << "," << value[1][2] << "," << value[2][2] << "," << value[3][2] << "," << ";" << RendererLogContext::NewLine;
        context << "            " << value[0][3] << "," << value[1][3] << "," << value[2][3] << "," << value[3][3] << "," << ";" << RendererLogContext::NewLine;
    }
}
