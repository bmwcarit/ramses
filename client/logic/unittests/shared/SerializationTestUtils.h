//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "generated/LuaScriptGen.h"
#include "generated/LuaInterfaceGen.h"
#include "generated/PropertyGen.h"
#include "generated/RamsesRenderPassBindingGen.h"
#include "generated/RamsesRenderGroupBindingGen.h"

namespace rlogic::internal
{
    class SerializationTestUtils
    {
    public:
        explicit SerializationTestUtils(flatbuffers::FlatBufferBuilder& builder)
            : m_builder(builder)
        {
        }

        flatbuffers::Offset<rlogic_serialization::Property> serializeTestProperty(
            std::string_view name,
            rlogic_serialization::EPropertyRootType type = rlogic_serialization::EPropertyRootType::Struct,
            bool withChildren = true,
            bool withErrors = false)
        {
            if (withErrors)
            {
                // Unnamed property -> causes errors down the hierarchy
                return rlogic_serialization::CreateProperty(
                    m_builder,
                    0);
            }

            if (withChildren)
            {
                auto child = rlogic_serialization::CreateProperty(
                    m_builder,
                    m_builder.CreateString("child"),
                    rlogic_serialization::EPropertyRootType::Primitive,
                    0,
                    rlogic_serialization::PropertyValue::float_s,
                    m_builder.CreateStruct(rlogic_serialization::float_s{0.42f}).Union()
                );
                return rlogic_serialization::CreateProperty(
                    m_builder,
                    m_builder.CreateString(name),
                    type,
                    m_builder.CreateVector(std::vector{child}));
            }

            return rlogic_serialization::CreateProperty(
                m_builder,
                m_builder.CreateString(name),
                type);
        }

        flatbuffers::Offset<rlogic_serialization::LuaScript> serializeTestScriptWithError()
        {
            return rlogic_serialization::CreateLuaScript(
                m_builder,
                0 // no base -> causes errors
            );
        }

        flatbuffers::Offset<rlogic_serialization::LuaInterface> serializeTestInterfaceWithError()
        {
            return rlogic_serialization::CreateLuaInterface(
                m_builder,
                rlogic_serialization::CreateLogicObject(m_builder,
                    m_builder.CreateString(""), // empty name -> causes errors
                    1u)
            );
        }

        flatbuffers::Offset<rlogic_serialization::LuaModule> serializeTestModule(bool withError = false)
        {
            return rlogic_serialization::CreateLuaModule(m_builder,
                rlogic_serialization::CreateLogicObject(m_builder,
                    withError ? 0 : m_builder.CreateString("moduleName"),
                    1u),
                m_builder.CreateString("{}"),
                m_builder.CreateVector(std::vector<flatbuffers::Offset<rlogic_serialization::LuaModuleUsage>>{}),
                m_builder.CreateVector(std::vector<uint8_t>{})
            );
        }

        flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding> serializeTestRenderPassBindingWithError()
        {
            return rlogic_serialization::CreateRamsesRenderPassBinding(
                m_builder,
                0 // no base -> causes errors
            );
        }

        flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding> serializeTestRenderGroupBindingWithError()
        {
            return rlogic_serialization::CreateRamsesRenderGroupBinding(
                m_builder,
                0 // no base -> causes errors
            );
        }

        flatbuffers::FlatBufferBuilder& m_builder;
    };
}
