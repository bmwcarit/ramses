//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internals/SolWrapper.h"
#include "ramses-logic/Collection.h"

namespace ramses
{
    class Property;
    class LogicNode;
    class LogicEngine;

    struct ConstPropertyWrapper
    {
        explicit ConstPropertyWrapper(const ramses::Property& property);

        sol::object getValue(sol::this_state L);

        sol::object toString(sol::this_state L);

        sol::object get(sol::stack_object key, sol::this_state L);

        const Property& m_property;
    };

    struct PropertyWrapper
    {
        explicit PropertyWrapper(ramses::Property& property);

        sol::object toString(sol::this_state L);

        sol::object getValue(sol::this_state L);

        void setValue(sol::stack_object value, sol::this_state L);

        sol::object get(sol::stack_object key, sol::this_state L);

        Property& m_property;
    };

    struct LogicNodeWrapper
    {
        explicit LogicNodeWrapper(LogicNode& logicNode);

        sol::object toString(sol::this_state L);

        sol::object get(sol::stack_object key, sol::this_state L);

        LogicNode& m_logicNode;
    };

    template<class T>
    struct NodeListIterator
    {
        using Container = Collection<T>;
        explicit NodeListIterator(Container collection)
            : m_it(collection.begin())
            , m_end(collection.end())
        {
        }

        sol::object call(sol::this_state L)
        {
            if (m_it != m_end)
            {
                auto obj = sol::object(L, sol::in_place, LogicNodeWrapper(**m_it));
                ++m_it;
                return obj;
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        typename Container::iterator m_it;
        typename Container::iterator m_end;
    };

    template<class T>
    struct NodeListWrapper
    {
        explicit NodeListWrapper(LogicEngine& logicEngine);

        sol::object get(sol::stack_object key, sol::this_state L);
        NodeListIterator<T> iterator();

        LogicNode* find(std::string_view key);
        Collection<T> collection();

        LogicEngine& m_logicEngine;
    };
}

