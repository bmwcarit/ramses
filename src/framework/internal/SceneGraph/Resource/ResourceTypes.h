//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/HeapArray.h"
#include "internal/Core/Common/StronglyTypedValue.h"
#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    using ResourceBlob = HeapArray<std::byte, struct ResourceBlobTag>;
    using CompressedResourceBlob = HeapArray<std::byte, struct CompressedResourceBlobTag>;

    enum class EResourceType
    {
        Invalid,
        VertexArray,
        IndexArray,
        Texture2D,
        Texture3D,
        TextureCube,
        Effect,
    };

    const std::array EResourceTypeNames =
    {
        "Invalid",
        "VertexArray",
        "IndexArray",
        "Texture2D",
        "Texture3D",
        "TextureCube",
        "Effect"
    };

    const size_t EResourceType_NUMBER_OF_ELEMENTS = EResourceTypeNames.size();
    static_assert(EnumTraits::VerifyElementCountIfSupported<EResourceType>(EResourceType_NUMBER_OF_ELEMENTS));

    ENUM_TO_STRING(EResourceType, EResourceTypeNames, EResourceType::Effect);
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EResourceType,
                                        "EResourceType",
                                        ramses::internal::EResourceTypeNames,
                                        ramses::internal::EResourceType::Effect);
