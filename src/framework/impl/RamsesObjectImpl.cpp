//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesObjectImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SerializationContext.h"
#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"

namespace ramses::internal
{
    RamsesObjectImpl::RamsesObjectImpl(ERamsesObjectType type, std::string_view name)
        : m_type(type)
        , m_name(name)
    {
    }

    RamsesObjectImpl::~RamsesObjectImpl() = default;

    ERamsesObjectType RamsesObjectImpl::getType() const
    {
        return m_type;
    }

    bool RamsesObjectImpl::isOfType(ERamsesObjectType type) const
    {
        return RamsesObjectTypeUtils::IsTypeMatchingBaseType(m_type, type);
    }

    const std::string& RamsesObjectImpl::getName() const
    {
        return m_name;
    }

    bool RamsesObjectImpl::setName(std::string_view name)
    {
        m_name = name;
        return true;
    }

    bool RamsesObjectImpl::setUserId(uint64_t highId, uint64_t lowId)
    {
        m_userId = { highId, lowId };
        return true;
    }

    std::pair<uint64_t, uint64_t> RamsesObjectImpl::getUserId() const
    {
        return m_userId;
    }

    std::string RamsesObjectImpl::getIdentificationString() const
    {
        if (m_userId.first != 0u || m_userId.second != 0u)
            return fmt::format("{} [{} UserId={:016X}{:016X}]", getName(), RamsesObjectTypeUtils::GetRamsesObjectTypeName(m_type), m_userId.first, m_userId.second);

        return fmt::format("{} [{}]", getName(), RamsesObjectTypeUtils::GetRamsesObjectTypeName(m_type));
    }

    const RamsesObject& RamsesObjectImpl::getRamsesObject() const
    {
        assert(m_ramsesObject != nullptr);
        return *m_ramsesObject;
    }

    RamsesObject& RamsesObjectImpl::getRamsesObject()
    {
        // non-const version of getRamsesObject cast to its const version to avoid duplicating code
        return const_cast<RamsesObject&>((const_cast<const RamsesObjectImpl&>(*this)).getRamsesObject());
    }

    void RamsesObjectImpl::setRamsesObject(RamsesObject& ramsesObject)
    {
        assert(m_ramsesObject == nullptr);
        m_ramsesObject = &ramsesObject;
    }

    bool RamsesObjectImpl::serialize(IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        outStream << serializationContext.getIDForObject(this);
        outStream << m_name;
        outStream << m_userId.first;
        outStream << m_userId.second;

        return true;
    }

    bool RamsesObjectImpl::deserialize(IInputStream& inStream, [[maybe_unused]] DeserializationContext& serializationContext)
    {
        inStream >> m_name;
        inStream >> m_userId.first;
        inStream >> m_userId.second;

        return true;
    }

    bool RamsesObjectImpl::resolveDeserializationDependencies([[maybe_unused]] DeserializationContext& serializationContext)
    {
        return true;
    }

    void RamsesObjectImpl::validate(ValidationReportImpl& report) const
    {
        // avoid double report
        if (report.addVisit(this))
        {
            onValidate(report);
            // validate dependencies
            for (auto* obj : report.getDependentObjects(this))
            {
                obj->validate(report);
            }
        }
    }
}
