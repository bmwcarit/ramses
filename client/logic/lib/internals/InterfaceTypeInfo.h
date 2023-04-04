//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"
#include "ramses-logic/EPropertyType.h"

namespace rlogic::internal
{
    // TODO Violin consider reworking this to use HierarchicalTypeData instead
    struct InterfaceTypeInfo
    {
        EPropertyType typeId = EPropertyType::Struct;
        size_t arraySize = 0u;
        sol::object complexType = sol::nil;
    };
}
