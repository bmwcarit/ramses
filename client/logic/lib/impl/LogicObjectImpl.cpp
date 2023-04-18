//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LogicObjectImpl.h"
#include "ramses-logic/LuaInterface.h"
#include "impl/LoggerImpl.h"
#include "internals/ErrorReporting.h"
#include "flatbuffers/flatbuffers.h"
#include "generated/LogicObjectGen.h"

namespace rlogic::internal
{
    LogicObjectImpl::LogicObjectImpl(std::string_view name, uint64_t id) noexcept
        : m_name(name)
        , m_id(id)
    {
    }

    LogicObjectImpl::~LogicObjectImpl() noexcept = default;

    std::string_view LogicObjectImpl::getName() const
    {
        return m_name;
    }

    bool LogicObjectImpl::setName(std::string_view name)
    {
        m_name = name;
        return true;
    }

    uint64_t LogicObjectImpl::getId() const
    {
        return m_id;
    }

    bool LogicObjectImpl::setUserId(uint64_t highId, uint64_t lowId)
    {
        m_userId = { highId, lowId };
        return true;
    }

    std::pair<uint64_t, uint64_t> LogicObjectImpl::getUserId() const
    {
        return m_userId;
    }

    std::string LogicObjectImpl::getIdentificationString() const
    {
        if (m_userId.first != 0u || m_userId.second != 0u)
            return fmt::format("{} [Id={} UserId={:016X}{:016X}]", m_name, m_id, m_userId.first, m_userId.second);

        return fmt::format("{} [Id={}]", m_name, m_id);
    }

    flatbuffers::Offset<rlogic_serialization::LogicObject> LogicObjectImpl::Serialize(const LogicObjectImpl& object, flatbuffers::FlatBufferBuilder& builder)
    {
        return rlogic_serialization::CreateLogicObject(builder,
            builder.CreateString(object.m_name),
            object.m_id,
            object.m_userId.first,
            object.m_userId.second
            );
    }

    bool LogicObjectImpl::Deserialize(const rlogic_serialization::LogicObject* object,
        std::string& name,
        uint64_t& id,
        uint64_t& userIdHigh,
        uint64_t& userIdLow,
        ErrorReporting& errorReporting)
    {
        if (!object)
        {
            errorReporting.add("Fatal error during loading of LogicObject base from serialized data: missing base table!", nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if (!object->name())
        {
            errorReporting.add("Fatal error during loading of LogicObject base from serialized data: missing name!", nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if (object->id() == 0u)
        {
            errorReporting.add("Fatal error during loading of LogicObject base from serialized data: missing or invalid ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        name = object->name()->string_view();
        id = object->id();
        userIdHigh = object->userIdHigh();
        userIdLow = object->userIdLow();

        return true;
    }

    void LogicObjectImpl::setLogicObject(LogicObject& obj)
    {
        assert(m_logicObject == nullptr);
        m_logicObject = &obj;
    }

    const LogicObject& LogicObjectImpl::getLogicObject() const
    {
        assert(m_logicObject != nullptr);
        return *m_logicObject;
    }

    LogicObject& LogicObjectImpl::getLogicObject()
    {
        assert(m_logicObject != nullptr);
        return *m_logicObject;
    }
}
