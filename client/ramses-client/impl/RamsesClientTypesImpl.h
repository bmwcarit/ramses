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
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"

template <>
struct fmt::formatter<ramses::TextureSwizzle> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses::TextureSwizzle& swizzle, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "TextureSwizzling:[{};{};{};{}]",
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)),
                              ramses_internal::EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)));
    }
};


template <>
struct fmt::formatter<ramses::ResourceFileDescription> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses::ResourceFileDescription& rhs, FormatContext& ctx)
    {
        const auto numResources = rhs.getNumberOfResources();
        fmt::format_to(ctx.out(), "Filename: {}; Resource count {}: [", rhs.getFilename(), numResources);
        for (uint32_t i = 0; i < numResources; ++i)
            fmt::format_to(ctx.out(), "{} ", rhs.getResource(i).getResourceId());
        return fmt::format_to(ctx.out(), "]");
    }
};

template <>
struct fmt::formatter<ramses::ResourceFileDescriptionSet> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses::ResourceFileDescriptionSet& rhs, FormatContext& ctx)
    {
        const auto numDescriptions = rhs.getNumberOfDescriptions();
        fmt::format_to(ctx.out(), "{} Resource File Descriptions: [", numDescriptions);
        for (uint32_t i = 0; i < numDescriptions; ++i)
            fmt::format_to(ctx.out(), "{}; ", rhs.getDescription(i));
        return fmt::format_to(ctx.out(), "]");
    }
};

template <>
struct fmt::formatter<ramses::resourceId_t> : public ramses_internal::SimpleFormatterBase
{
    template<typename FormatContext>
    auto format(const ramses::resourceId_t& res, FormatContext& ctx)
    {
        return fmt::format_to(ctx.out(), "0x{:016X}:{:016X}", res.highPart, res.lowPart);
    }
};

#endif
