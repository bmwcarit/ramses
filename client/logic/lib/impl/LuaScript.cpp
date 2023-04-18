//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LuaScript.h"
#include "impl/LuaScriptImpl.h"

namespace rlogic
{
    LuaScript::LuaScript(std::unique_ptr<internal::LuaScriptImpl> impl) noexcept
        : LogicNode(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_script{ static_cast<internal::LuaScriptImpl&>(LogicNode::m_impl) }
    {
    }

    LuaScript::~LuaScript() noexcept = default;
}
