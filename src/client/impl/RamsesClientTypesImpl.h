//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/EDataType.h"
#include "ramses/client/TextureSwizzle.h"
#include "impl/TextureUtils.h"
#include "impl/TextureEnumsImpl.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/Core/Common/TypedMemoryHandle.h"

template <>
struct fmt::formatter<ramses::TextureSwizzle> : public ramses::internal::SimpleFormatterBase
{
    template<typename FormatContext>
    constexpr auto format(const ramses::TextureSwizzle& swizzle, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "TextureSwizzling:[{};{};{};{}]",
                              ramses::EnumToString(swizzle.channelRed),
                              ramses::EnumToString(swizzle.channelGreen),
                              ramses::EnumToString(swizzle.channelBlue),
                              ramses::EnumToString(swizzle.channelAlpha));
    }
};

namespace ramses
{
    const std::array DataTypeNames =
    {
        "UInt16",
        "UInt32",
        "Float",
        "Vector2F",
        "Vector3F",
        "Vector4F",
        "ByteBlob",
        "Bool",
        "Int32",
        "Vector2I",
        "Vector3I",
        "Vector4I",
        "Matrix22F",
        "Matrix33F",
        "Matrix44F",
        "TextureSampler2D",
        "TextureSampler2DMS",
        "TextureSampler3D",
        "TextureSamplerCube",
        "TextureSamplerExternal",
    };

    namespace internal
    {
        struct SceneObjectRegistryHandleTag {};
        using SceneObjectRegistryHandle = ramses::internal::TypedMemoryHandle<SceneObjectRegistryHandleTag>;
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::EDataType, "EDataType", ramses::DataTypeNames, ramses::EDataType::TextureSamplerExternal);
