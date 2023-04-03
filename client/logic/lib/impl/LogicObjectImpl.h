//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <string>

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic_serialization
{
    struct LogicObject;
}

namespace rlogic
{
    class LogicObject;
}

namespace rlogic::internal
{
    class ErrorReporting;

    class LogicObjectImpl
    {
    public:
        explicit LogicObjectImpl(std::string_view name, uint64_t id) noexcept;
        virtual ~LogicObjectImpl() noexcept;
        LogicObjectImpl(const LogicObjectImpl&) = delete;
        LogicObjectImpl& operator=(const LogicObjectImpl&) = delete;

        [[nodiscard]] std::string_view getName() const;
        [[nodiscard]] uint64_t getId() const;
        bool setName(std::string_view name);
        bool setUserId(uint64_t highId, uint64_t lowId);
        [[nodiscard]] std::pair<uint64_t, uint64_t> getUserId() const;

        [[nodiscard]] std::string getIdentificationString() const;

        void setLogicObject(LogicObject& obj);
        [[nodiscard]] const LogicObject& getLogicObject() const;
        [[nodiscard]] LogicObject& getLogicObject();

    protected:
        static flatbuffers::Offset<rlogic_serialization::LogicObject> Serialize(const LogicObjectImpl& object, flatbuffers::FlatBufferBuilder& builder);
        static bool Deserialize(const rlogic_serialization::LogicObject* object,
            std::string& name,
            uint64_t& id,
            uint64_t& userIdHigh,
            uint64_t& userIdLow,
            ErrorReporting& errorReporting);

    private:
        std::string m_name;
        uint64_t    m_id;
        std::pair<uint64_t, uint64_t> m_userId{ 0u, 0u };
        LogicObject* m_logicObject = nullptr;
    };
}
