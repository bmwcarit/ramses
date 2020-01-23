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

namespace ramses
{
    class ResourceFileDescription;
    class ResourceFileDescriptionSet;

    inline ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& os, const TextureSwizzle& swizzle)
    {
        os << "TextureSwizzling:[" << ramses_internal::EnumToString(TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)) << ";";
        os << ramses_internal::EnumToString(TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)) << ";";
        os << ramses_internal::EnumToString(TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)) << ";";
        os << ramses_internal::EnumToString(TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)) << "]";
        return os;
    }

    inline ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& lhs, resourceId_t const& rhs)
    {
        lhs.setHexadecimalOutputFormat(ramses_internal::StringOutputStream::EHexadecimalType_HexLeadingZeros);
        lhs << "0x" << rhs.highPart << ":" << rhs.lowPart;
        lhs.setHexadecimalOutputFormat(ramses_internal::StringOutputStream::EHexadecimalType_NoHex);
        return lhs;
    }

    ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& lhs, const ResourceFileDescription& rhs);
    ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& lhs, const ResourceFileDescriptionSet& rhs);
}

#endif
