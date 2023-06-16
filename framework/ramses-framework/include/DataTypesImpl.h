//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once


#include "ramses-framework-api/DataTypes.h"
#include "PlatformAbstraction/FmtBase.h"

template <> struct fmt::formatter<glm::vec4> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::vec4& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {} {}]", m.x, m.y, m.z, m.w);
    }
};

template <> struct fmt::formatter<glm::vec3> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::vec3& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {}]", m.x, m.y, m.z);
    }
};

template <> struct fmt::formatter<glm::vec2> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::vec2& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {}]", m.x, m.y);
    }
};

template <> struct fmt::formatter<glm::ivec4> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::ivec4& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {} {}]", m.x, m.y, m.z, m.w);
    }
};

template <> struct fmt::formatter<glm::ivec3> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::ivec3& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {} {}]", m.x, m.y, m.z);
    }
};

template <> struct fmt::formatter<glm::ivec2> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext> constexpr auto format(const glm::ivec2& m, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "[{} {}]", m.x, m.y);
    }
};

