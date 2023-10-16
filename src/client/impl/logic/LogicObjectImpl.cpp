//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/LogicObjectImpl.h"
#include "ramses/client/logic/LuaInterface.h"
#include "fmt/format.h"
#include "impl/ErrorReporting.h"
#include "flatbuffers/flatbuffers.h"
#include "internal/logic/flatbuffers/generated/LogicObjectGen.h"

namespace ramses::internal
{
    LogicObjectImpl::LogicObjectImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept
        : SceneObjectImpl{ scene, ERamsesObjectType::LogicObject, name }
    {
        if (id.isValid()) // set ID from deserialization if created object during loading
            m_sceneObjectId = id;
    }

    flatbuffers::Offset<rlogic_serialization::LogicObject> LogicObjectImpl::Serialize(const LogicObjectImpl& object, flatbuffers::FlatBufferBuilder& builder)
    {
        return rlogic_serialization::CreateLogicObject(builder,
            builder.CreateString(object.getName()),
            object.getSceneObjectId().getValue(),
            object.getUserId().first,
            object.getUserId().second
            );
    }

    bool LogicObjectImpl::Deserialize(const rlogic_serialization::LogicObject* object,
        std::string& name,
        sceneObjectId_t& id,
        uint64_t& userIdHigh,
        uint64_t& userIdLow,
        ErrorReporting& errorReporting)
    {
        if (!object)
        {
            errorReporting.set("Fatal error during loading of LogicObject base from serialized data: missing base table!", nullptr);
            return false;
        }

        if (!object->name())
        {
            errorReporting.set("Fatal error during loading of LogicObject base from serialized data: missing name!", nullptr);
            return false;
        }

        if (object->id() == 0u)
        {
            errorReporting.set("Fatal error during loading of LogicObject base from serialized data: missing or invalid logic object ID!", nullptr);
            return false;
        }

        name = object->name()->string_view();
        id = sceneObjectId_t{ object->id() };
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
