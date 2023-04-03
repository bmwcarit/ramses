//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LuaModule.h"
#include "impl/LuaModuleImpl.h"

namespace rlogic
{
    LuaModule::LuaModule(std::unique_ptr<internal::LuaModuleImpl> impl) noexcept
        : LogicObject(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_impl{ static_cast<internal::LuaModuleImpl&>(*LogicObject::m_impl) }
    {
    }

    LuaModule::~LuaModule() noexcept = default;
}
