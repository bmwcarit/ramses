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

namespace rlogic::internal
{
    class LuaInterfaceImpl;
}

namespace rlogic
{
    class Property;

    /**
    * LuaInterface encapsulates forms an additional layer to a logic network (whole or its parts) and it exposes only relevant
    * inputs to the user of that network. Those input variables can be used to represent scenes in an abstract way.
    * LuaInterface instances are created by the #rlogic::LogicEngine class.
    *
    * A LuaInterface can be created from Lua source code which must fulfill same requirements as #rlogic::LuaScript, except for the following:
    *
    * - A Lua interface contains only one global function: interface()
    * - The interface() function takes only one parameter that represents both inputs and outputs, e.g., interface(INOUT_PARAMS)
    *
    * Violating any of these requirements will result in errors, which can be obtained by calling
    * #rlogic::LogicEngine::getErrors().
    * For other than interface definition purposes, see #rlogic::LuaModule and #rlogic::LuaScript for details.
    *
    * See also the full documentation at https://ramses-logic.readthedocs.io/en/latest/api.html for more details on Lua and
    * its interaction with C++.
    */
    class LuaInterface : public LogicNode
    {
    public:

        /**
        * Constructor of LuaInterface. User is not supposed to call this - interface objects are created by other factory classes
        *
        * @param impl implementation details of the interface
        */
        explicit LuaInterface(std::unique_ptr<internal::LuaInterfaceImpl> impl) noexcept;

        /**
        * Destructor of LuaInterface
        */
        ~LuaInterface() noexcept override;

        /**
        * Copy Constructor of LuaInterface is deleted because scripts are not supposed to be copied
        *
        * @param other interface to copy from
        */
        LuaInterface(const LuaInterface& other) = delete;

        /**
        * Move Constructor of LuaInterface is deleted because scripts are not supposed to be moved
        *
        * @param other interface to move from
        */
        LuaInterface(LuaInterface&& other) = delete;

        /**
        * Assignment operator of LuaInterface is deleted because scripts are not supposed to be copied
        *
        * @param other interface to assign from
        */
        LuaInterface& operator=(const LuaInterface& other) = delete;

        /**
        * Move assignment operator of LuaInterface is deleted because scripts are not supposed to be moved
        *
        * @param other interface to move from
        */
        LuaInterface& operator=(LuaInterface&& other) = delete;

        /**
        * Implementation detail of LuaInterface
        */
        internal::LuaInterfaceImpl& m_interface;
    };
}
