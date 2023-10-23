//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "fmt/format.h"

namespace ramses::internal
{
    enum class ERendererToClientEventType : uint32_t
    {
        SceneReferencingEvent, // information about scene referencing status from renderer back to client
        ResourcesAvailableAtRendererEvent // information about available resources which do not need to be sent
    };

    inline const char* EnumToString(ERendererToClientEventType val)
    {
        switch (val)
        {
        case ERendererToClientEventType::SceneReferencingEvent: return "ERendererToClientEventType::SceneReferencingEvent";
        case ERendererToClientEventType::ResourcesAvailableAtRendererEvent: return "ERendererToClientEventType::ResourcesAvailableAtRendererEvent";
        }
        return "ERendererEventType::<UNKNOWN>";
    }

}

template <>
    struct fmt::formatter<ramses::internal::ERendererToClientEventType> {
        template<typename ParseContext>
        constexpr auto parse(ParseContext& ctx) {
            return ctx.begin();
        }
        template<typename FormatContext>
            constexpr auto format(const ramses::internal::ERendererToClientEventType& thetype, FormatContext& ctx) {
                return fmt::format_to(ctx.out(), "{}",
                    EnumToString(thetype));
        }
};

