//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/ELuaSavingMode.h"

namespace ramses::internal
{
    class ApiObjects;

    class ApiObjectsSerializedSize
    {
    public:
        [[nodiscard]] static std::size_t GetTotalSerializedSize(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode);

        template <typename T>
        [[nodiscard]] static std::size_t GetSerializedSize(const ApiObjects& apiObjects, ELuaSavingMode luaSavingMode = ELuaSavingMode::SourceAndByteCode);
    };
}
