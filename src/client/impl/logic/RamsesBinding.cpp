//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/RamsesBinding.h"

#include "impl/logic/RamsesBindingImpl.h"

namespace ramses
{
    RamsesBinding::RamsesBinding(std::unique_ptr<internal::RamsesBindingImpl> impl) noexcept
        : LogicNode(std::move(impl))
    {
    }
}
