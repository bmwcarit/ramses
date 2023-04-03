//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerLuaTypes.h"
#include "LogicViewer.h"
#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LogicNode.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaInterface.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/Property.h"
#include "internals/SolHelper.h"
#include "fmt/format.h"

namespace rlogic
{
    namespace
    {
        void setPropertyValue(Property& prop, const sol::object& val)
        {
            switch (prop.getType())
            {
            case rlogic::EPropertyType::Int32: {
                prop.set(val.as<int32_t>());
                break;
            }
            case rlogic::EPropertyType::Int64: {
                prop.set(val.as<int64_t>());
                break;
            }
            case rlogic::EPropertyType::Struct: {
                auto tbl = val.as<sol::table>();
                for (const auto& item : tbl)
                {
                    auto      ki  = item.first.as<std::string>();
                    Property* sub = prop.getChild(ki);
                    if (sub == nullptr)
                        sol_helper::throwSolException("Property not found in struct: {}", ki);
                    setPropertyValue(*sub, item.second);
                }
                break;
            }
            case rlogic::EPropertyType::Float:
                prop.set(val.as<float>());
                break;
            case rlogic::EPropertyType::Vec2f:
                prop.set(val.as<vec2f>());
                break;
            case rlogic::EPropertyType::Vec3f:
                prop.set(val.as<vec3f>());
                break;
            case rlogic::EPropertyType::Vec4f:
                prop.set(val.as<vec4f>());
                break;
            case rlogic::EPropertyType::Vec2i:
                prop.set(val.as<vec2i>());
                break;
            case rlogic::EPropertyType::Vec3i:
                prop.set(val.as<vec3i>());
                break;
            case rlogic::EPropertyType::Vec4i:
                prop.set(val.as<vec4i>());
                break;
            case rlogic::EPropertyType::Bool:
                prop.set(val.as<bool>());
                break;
            case rlogic::EPropertyType::String:
                prop.set(val.as<std::string>());
                break;
            case rlogic::EPropertyType::Array:
                auto tbl = val.as<sol::table>();
                for (const auto& item : tbl)
                {
                    auto      ki  = item.first.as<int>();
                    Property* sub = prop.getChild(ki - 1);
                    if (sub == nullptr)
                        sol_helper::throwSolException("index {} out of bounds for array {}[{}]", ki, prop.getName(), prop.getChildCount());
                    setPropertyValue(*sub, item.second);
                }
                break;
            }
        }

        sol::object getPropertyValue(const rlogic::Property& prop, sol::this_state L)
        {
            switch (prop.getType())
            {
            case rlogic::EPropertyType::Int32:
                return sol::object(L, sol::in_place, prop.get<int32_t>());
            case rlogic::EPropertyType::Int64:
                return sol::object(L, sol::in_place, prop.get<int64_t>());
            case rlogic::EPropertyType::Float:
                return sol::object(L, sol::in_place, prop.get<float>());
            case rlogic::EPropertyType::Vec2f:
                return sol::object(L, sol::in_place, prop.get<vec2f>());
            case rlogic::EPropertyType::Vec3f:
                return sol::object(L, sol::in_place, prop.get<vec3f>());
            case rlogic::EPropertyType::Vec4f:
                return sol::object(L, sol::in_place, prop.get<vec4f>());
            case rlogic::EPropertyType::Vec2i:
                return sol::object(L, sol::in_place, prop.get<vec2i>());
            case rlogic::EPropertyType::Vec3i:
                return sol::object(L, sol::in_place, prop.get<vec3i>());
            case rlogic::EPropertyType::Vec4i:
                return sol::object(L, sol::in_place, prop.get<vec4i>());
            case rlogic::EPropertyType::Bool:
                return sol::object(L, sol::in_place, prop.get<bool>());
            case rlogic::EPropertyType::String:
                return sol::object(L, sol::in_place, prop.get<std::string>());
            case rlogic::EPropertyType::Array:
            case rlogic::EPropertyType::Struct:
                // no values for array and struct
                break;
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        template<bool Const>
        sol::object getChildProperty(rlogic::Property& prop, sol::stack_object key, sol::this_state L)
        {
            using ChildWrapper = std::conditional_t<Const, ConstPropertyWrapper, PropertyWrapper>;
            auto strKey = key.as<sol::optional<std::string>>();
            Property* child  = nullptr;
            if (strKey)
            {
                child = prop.getChild(*strKey);
            }
            else
            {
                auto intKey = key.as<sol::optional<int>>();
                if (intKey)
                {
                    // expect 1-based index by lua convention
                    auto intKeyValue = static_cast<size_t>(*intKey);
                    if (intKeyValue != 0u)
                    {
                        child = prop.getChild(intKeyValue - 1u);
                    }
                }
            }
            if (child)
            {
                return sol::object(L, sol::in_place, ChildWrapper(*child));
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }
    } // namespace

    ConstPropertyWrapper::ConstPropertyWrapper(const rlogic::Property& property)
        : m_property(property)
    {
    }

    sol::object ConstPropertyWrapper::toString(sol::this_state L)
    {
        std::string name = "ConstProperty: ";
        name += m_property.getName();
        return sol::object(L, sol::in_place, name);
    }

    sol::object ConstPropertyWrapper::getValue(sol::this_state L)
    {
        return getPropertyValue(m_property, L);
    }

    sol::object ConstPropertyWrapper::get(sol::stack_object key, sol::this_state L)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) avoid code duplication
        return getChildProperty<true>(const_cast<Property&>(m_property), key, L);
    }

    PropertyWrapper::PropertyWrapper(rlogic::Property& property)
        : m_property(property)
    {
    }

    sol::object PropertyWrapper::toString(sol::this_state L)
    {
        std::string name = "Property: ";
        name += m_property.getName();
        return sol::object(L, sol::in_place, name);
    }

    sol::object PropertyWrapper::getValue(sol::this_state L)
    {
        return getPropertyValue(m_property, L);
    }

    void PropertyWrapper::setValue(sol::stack_object value, sol::this_state /*L*/)
    {
        setPropertyValue(m_property, value);
    }

    sol::object PropertyWrapper::get(sol::stack_object key, sol::this_state L)
    {
        return getChildProperty<false>(m_property, key, L);
    }


    LogicNodeWrapper::LogicNodeWrapper(LogicNode& logicNode)
        : m_logicNode(logicNode)
    {
    }

    sol::object LogicNodeWrapper::toString(sol::this_state L)
    {
        std::string name = "LogicNode: ";
        name += m_logicNode.getName();
        return sol::object(L, sol::in_place, name);
    }

    sol::object LogicNodeWrapper::get(sol::stack_object key, sol::this_state L)
    {
        auto strKey = key.as<sol::optional<std::string>>();
        sol::object retval;
        if (strKey)
        {
            auto* inputs = m_logicNode.getInputs();
            if ((inputs != nullptr) && (strKey == LogicViewer::ltnIN))
            {
                retval = sol::object(L, sol::in_place, PropertyWrapper(*inputs));
            }
            else
            {
                auto* outputs = m_logicNode.getOutputs();
                if ((outputs != nullptr) && (strKey == LogicViewer::ltnOUT))
                {
                    retval = sol::object(L, sol::in_place, ConstPropertyWrapper(*outputs));
                }
            }
        }
        if (!retval.valid())
        {
            retval = sol::object(L, sol::in_place, sol::lua_nil);
        }
        return retval;
    }

    template<class T>
    NodeListWrapper<T>::NodeListWrapper(LogicEngine& logicEngine)
        : m_logicEngine(logicEngine)
    {
    }

    template<class T>
    sol::object NodeListWrapper<T>::get(sol::stack_object key, sol::this_state L)
    {
        auto strKey = key.as<sol::optional<std::string>>();
        rlogic::LogicNode* node = nullptr;
        if (strKey)
        {
            node = find(*strKey);
        }
        else
        {
            auto intKey = key.as<sol::optional<int>>();
            if (intKey)
            {
                auto* obj = m_logicEngine.findLogicObjectById(static_cast<uint64_t>(*intKey));
                if (obj != nullptr)
                {
                    node = obj->as<rlogic::LogicNode>();
                }
            }
        }

        if (node != nullptr)
        {
            return sol::object(L, sol::in_place, LogicNodeWrapper(*node));
        }
        return sol::object(L, sol::in_place, sol::lua_nil);
    }

    template <class T>
    LogicNode* NodeListWrapper<T>::find(std::string_view key)
    {
        return m_logicEngine.findByName<T>(key);
    }

    template <class T>
    NodeListIterator<T> NodeListWrapper<T>::iterator()
    {
        return NodeListIterator(collection());
    }

    template <class T>
    Collection<T> NodeListWrapper<T>::collection()
    {
        return m_logicEngine.getCollection<T>();
    }

    template struct NodeListWrapper<AnimationNode>;
    template struct NodeListWrapper<TimerNode>;
    template struct NodeListWrapper<LuaScript>;
    template struct NodeListWrapper<LuaInterface>;
    template struct NodeListWrapper<RamsesNodeBinding>;
    template struct NodeListWrapper<RamsesAppearanceBinding>;
    template struct NodeListWrapper<RamsesCameraBinding>;
    template struct NodeListWrapper<RamsesRenderPassBinding>;
    template struct NodeListWrapper<RamsesRenderGroupBinding>;
    template struct NodeListWrapper<RamsesMeshNodeBinding>;
    template struct NodeListWrapper<AnchorPoint>;
    template struct NodeListWrapper<SkinBinding>;
} // namespace rlogic

