//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LuaInterface.h"
#include "impl/LuaInterfaceImpl.h"

namespace rlogic
{
    LuaInterface::LuaInterface(std::unique_ptr<internal::LuaInterfaceImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_interface{ static_cast<internal::LuaInterfaceImpl&>(LogicNode::m_impl) }
    {
    }

    LuaInterface::~LuaInterface() noexcept = default;
}
