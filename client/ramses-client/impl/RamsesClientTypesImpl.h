//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESCLIENTTYPESIMPL_H
#define RAMSES_RAMSESCLIENTTYPESIMPL_H

#include "Collections/StringOutputStream.h"
#include "ramses-client-api/TextureSwizzle.h"
#include "SceneAPI/TextureEnums.h"
#include "TextureUtils.h"
#include "ramses-client-api/EDataType.h"

template <>
struct fmt::formatter<ramses::TextureSwizzle> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::TextureSwizzle& swizzle, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "TextureSwizzling:[{};{};{};{}]",
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)));
    }
};

namespace ramses
{
    static const char* DataTypeNames[] =
    {
        "UINT16",
        "UINT32",
        "FLOAT",
        "VECTOR2F",
        "VECTOR3F",
        "VECTOR4F",
    };
}

MAKE_ENUM_CLASS_PRINTABLE_NO_EXTRA_LAST(ramses::EDataType, "EDataType", ramses::DataTypeNames, ramses::EDataType::Vector4F)

#endif
