//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/SceneObjectImpl.h"
#include <string>

namespace flatbuffers
{
    template<typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace rlogic_serialization
{
    struct LogicObject;
}

namespace ramses
{
    class LogicObject;
}

namespace ramses::internal
{
    class ErrorReporting;
    class SceneImpl;

    class LogicObjectImpl : public SceneObjectImpl
    {
    public:
        explicit LogicObjectImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id) noexcept;

        void setLogicObject(LogicObject& obj);
        [[nodiscard]] const LogicObject& getLogicObject() const;
        [[nodiscard]] LogicObject& getLogicObject();

        // RamsesObject implementation
        void deinitializeFrameworkData() final override { /*logic has no internal framework data*/ }

    protected:
        // Logic objects are validated by LogicEngineImpl
        void onValidate(ValidationReportImpl& /*report*/) const final override {}

        static flatbuffers::Offset<rlogic_serialization::LogicObject> Serialize(const LogicObjectImpl& object, flatbuffers::FlatBufferBuilder& builder);
        static bool Deserialize(const rlogic_serialization::LogicObject* object,
            std::string& name,
            sceneObjectId_t& id,
            uint64_t& userIdHigh,
            uint64_t& userIdLow,
            ErrorReporting& errorReporting);

    private:
        LogicObject* m_logicObject = nullptr;
    };
}
