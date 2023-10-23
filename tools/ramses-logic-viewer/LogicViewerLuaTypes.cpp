//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicViewerLuaTypes.h"
#include "LogicViewer.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicNode.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/Property.h"
#include "internal/logic/SolHelper.h"
#include "fmt/format.h"
#include "glm/gtx/range.hpp"

namespace glm
{
    template <int N, typename T, qualifier Q>
    int sol_lua_push(sol::types<vec<N, T, Q>> /*unused*/, lua_State* L, const vec<N, T, Q>& value)
    {
        auto vectable = sol::table::create(L, N);
        for (const auto& v : value)
        {
            vectable.add(v);
        }
        int amount = sol::stack::push(L, vectable);
        return amount;
    }

    template <int N, typename T, qualifier Q>
    vec<N, T, Q> sol_lua_get(sol::types<vec<N, T, Q>> /*unused*/, lua_State* L, int index, sol::stack::record& tracking)
    {
        sol::lua_table vectable = sol::stack::get<sol::lua_table>(L, index, tracking);
        vec<N, T, Q> result{};
        for (glm::length_t i = 0; i < N; ++i)
        {
            result[i] = vectable[i + 1];
        }
        return result;
    }
}

namespace ramses
{
    namespace
    {
        void setPropertyValue(Property& prop, const sol::object& val)
        {
            switch (prop.getType())
            {
            case ramses::EPropertyType::Int32: {
                prop.set(val.as<int32_t>());
                break;
            }
            case ramses::EPropertyType::Int64: {
                prop.set(val.as<int64_t>());
                break;
            }
            case ramses::EPropertyType::Struct: {
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
            case ramses::EPropertyType::Float:
                prop.set(val.as<float>());
                break;
            case ramses::EPropertyType::Vec2f:
                prop.set(val.as<vec2f>());
                break;
            case ramses::EPropertyType::Vec3f:
                prop.set(val.as<vec3f>());
                break;
            case ramses::EPropertyType::Vec4f:
                prop.set(val.as<vec4f>());
                break;
            case ramses::EPropertyType::Vec2i:
                prop.set(val.as<vec2i>());
                break;
            case ramses::EPropertyType::Vec3i:
                prop.set(val.as<vec3i>());
                break;
            case ramses::EPropertyType::Vec4i:
                prop.set(val.as<vec4i>());
                break;
            case ramses::EPropertyType::Bool:
                prop.set(val.as<bool>());
                break;
            case ramses::EPropertyType::String:
                prop.set(val.as<std::string>());
                break;
            case ramses::EPropertyType::Array:
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

        sol::object getPropertyValue(const ramses::Property& prop, sol::this_state L)
        {
            switch (prop.getType())
            {
            case ramses::EPropertyType::Int32:
                return sol::object(L, sol::in_place, prop.get<int32_t>());
            case ramses::EPropertyType::Int64:
                return sol::object(L, sol::in_place, prop.get<int64_t>());
            case ramses::EPropertyType::Float:
                return sol::object(L, sol::in_place, prop.get<float>());
            case ramses::EPropertyType::Vec2f:
                return sol::object(L, sol::in_place, prop.get<vec2f>());
            case ramses::EPropertyType::Vec3f:
                return sol::object(L, sol::in_place, prop.get<vec3f>());
            case ramses::EPropertyType::Vec4f:
                return sol::object(L, sol::in_place, prop.get<vec4f>());
            case ramses::EPropertyType::Vec2i:
                return sol::object(L, sol::in_place, prop.get<vec2i>());
            case ramses::EPropertyType::Vec3i:
                return sol::object(L, sol::in_place, prop.get<vec3i>());
            case ramses::EPropertyType::Vec4i:
                return sol::object(L, sol::in_place, prop.get<vec4i>());
            case ramses::EPropertyType::Bool:
                return sol::object(L, sol::in_place, prop.get<bool>());
            case ramses::EPropertyType::String:
                return sol::object(L, sol::in_place, prop.get<std::string>());
            case ramses::EPropertyType::Array:
            case ramses::EPropertyType::Struct:
                // no values for array and struct
                break;
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        sol::object getChildProperty(ramses::Property& prop, sol::stack_object key, sol::this_state L)
        {
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
                return sol::object(L, sol::in_place, PropertyWrapper(*child));
            }
            return sol::object(L, sol::in_place, sol::lua_nil);
        }
    } // namespace

    PropertyWrapper::PropertyWrapper(ramses::Property& property)
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
        return getChildProperty(m_property, key, L);
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
                    retval = sol::object(L, sol::in_place, PropertyWrapper(*outputs));
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
        ramses::LogicNode* node = nullptr;
        if (strKey)
        {
            node = find(*strKey);
        }
        else
        {
            auto intKey = key.as<sol::optional<int>>();
            if (intKey)
            {
                auto* obj = m_logicEngine.findObject(sceneObjectId_t{ static_cast<uint64_t>(*intKey) });
                if (obj != nullptr)
                {
                    node = obj->template as<ramses::LogicNode>();
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
        return m_logicEngine.findObject<T>(key);
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
    template struct NodeListWrapper<NodeBinding>;
    template struct NodeListWrapper<AppearanceBinding>;
    template struct NodeListWrapper<CameraBinding>;
    template struct NodeListWrapper<RenderPassBinding>;
    template struct NodeListWrapper<RenderGroupBinding>;
    template struct NodeListWrapper<MeshNodeBinding>;
    template struct NodeListWrapper<AnchorPoint>;
    template struct NodeListWrapper<SkinBinding>;
} // namespace ramses

