//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LuaModule.h"
#include "impl/logic/LuaModuleImpl.h"

namespace ramses
{
    LuaModule::LuaModule(std::unique_ptr<internal::LuaModuleImpl> impl) noexcept
        : LogicObject(std::move(impl))
        /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast) */
        , m_impl{ static_cast<internal::LuaModuleImpl&>(LogicObject::m_impl) }
    {
    }

    internal::LuaModuleImpl& LuaModule::impl()
    {
        return m_impl;
    }

    const internal::LuaModuleImpl& LuaModule::impl() const
    {
        return m_impl;
    }
}
