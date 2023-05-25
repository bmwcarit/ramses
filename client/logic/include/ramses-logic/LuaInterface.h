//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/LogicNode.h"

#include <string>
#include <memory>
#include <functional>

namespace ramses::internal
{
    class LuaInterfaceImpl;
}

namespace ramses
{
    class Property;

    /**
    * LuaInterface encapsulates forms an additional layer to a logic network (whole or its parts) and it exposes only relevant
    * inputs to the user of that network. Those input variables can be used to represent scenes in an abstract way.
    * LuaInterface instances are created by the #ramses::LogicEngine class.
    *
    * A LuaInterface can be created from Lua source code which must fulfill same requirements as #ramses::LuaScript, except for the following:
    *
    * - A Lua interface contains only one global function: interface()
    * - The interface() function takes only one parameter that represents both inputs and outputs, e.g., interface(INOUT_PARAMS)
    *
    * Violating any of these requirements will result in errors, which can be obtained by calling
    * #ramses::LogicEngine::getErrors().
    * For other than interface definition purposes, see #ramses::LuaModule and #ramses::LuaScript for details.
    *
    * See also the full documentation at https://ramses-logic.readthedocs.io/en/latest/api.html for more details on Lua and
    * its interaction with C++.
    */
    class LuaInterface : public LogicNode
    {
    public:
        /**
        * Implementation detail of LuaInterface
        */
        internal::LuaInterfaceImpl& m_interface;

    protected:
        /**
        * Constructor of LuaInterface. User is not supposed to call this - interface objects are created by other factory classes
        *
        * @param impl implementation details of the interface
        */
        explicit LuaInterface(std::unique_ptr<internal::LuaInterfaceImpl> impl) noexcept;

        friend class internal::ApiObjects;
    };
}
