//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/framework/Issue.h"
#include "impl/APILoggingMacros.h"
#include "impl/ValidationReportImpl.h"

#include <string_view>
#include <string>

namespace ramses
{
    class RamsesObject;
}

namespace ramses::internal
{
    class IOutputStream;
    class IInputStream;
    class SerializationContext;
    class DeserializationContext;

    class RamsesObjectImpl
    {
    public:
        explicit RamsesObjectImpl(ERamsesObjectType type, std::string_view name);
        virtual ~RamsesObjectImpl();

        [[nodiscard]] ERamsesObjectType      getType() const;
        [[nodiscard]] bool                   isOfType(ERamsesObjectType type) const;
        [[nodiscard]] const std::string&     getName() const;
        [[nodiscard]] virtual bool           setName(std::string_view name);
        [[nodiscard]] const RamsesObject&    getRamsesObject() const;
        [[nodiscard]] RamsesObject&          getRamsesObject();

        bool setUserId(uint64_t highId, uint64_t lowId);
        [[nodiscard]] std::pair<uint64_t, uint64_t> getUserId() const;
        [[nodiscard]] virtual std::string getIdentificationString() const;

        void setRamsesObject(RamsesObject& ramsesObject);

        virtual bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const;
        virtual bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext);
        virtual bool resolveDeserializationDependencies(DeserializationContext& serializationContext);

        virtual void deinitializeFrameworkData() = 0;

        void validate(ValidationReportImpl& report) const;
        virtual void onValidate(ValidationReportImpl& /*report*/) const {};

    private:
        ERamsesObjectType m_type;
        std::string m_name;
        std::pair<uint64_t, uint64_t> m_userId{ 0u, 0u };

        RamsesObject* m_ramsesObject = nullptr;
    };
}
