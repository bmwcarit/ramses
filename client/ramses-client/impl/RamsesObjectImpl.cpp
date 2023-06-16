//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesObjectImpl.h"
#include "NodeImpl.h"
#include "IRamsesObjectRegistry.h"
#include "RamsesObjectTypeUtils.h"
#include "SerializationContext.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Collections/StringOutputStream.h"

namespace ramses
{
    RamsesObjectImpl::RamsesObjectImpl(ERamsesObjectType type, std::string_view name)
        : m_type(type)
        , m_name(name)
    {
    }

    RamsesObjectImpl::~RamsesObjectImpl()
    {
    }

    void RamsesObjectImpl::setObjectRegistry(IRamsesObjectRegistry& objectRegistry)
    {
        m_objectRegistry = &objectRegistry;
    }

    void RamsesObjectImpl::setObjectRegistryHandle(RamsesObjectHandle handle)
    {
        m_objectRegistryHandle = handle;
    }

    RamsesObjectHandle RamsesObjectImpl::getObjectRegistryHandle() const
    {
        return m_objectRegistryHandle;
    }

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

    status_t RamsesObjectImpl::setName(RamsesObject& object, std::string_view name)
    {
        std::string newName{name};
        if (m_objectRegistry)
        {
            // updateName must be called before m_name is changed
            m_objectRegistry->updateName(object, newName);
        }
        std::swap(m_name, newName);

        return StatusOK;
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

    ramses::status_t RamsesObjectImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        outStream << serializationContext.getIDForObject(this);
        outStream << m_name;

        return StatusOK;
    }

    status_t RamsesObjectImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        UNUSED(serializationContext);
        inStream >> m_name;

        return StatusOK;
    }

    status_t RamsesObjectImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        UNUSED(serializationContext);
        return StatusOK;
    }

    status_t RamsesObjectImpl::validate() const
    {
        const status_t status = StatusObjectImpl::validate();
        std::string message{ fmt::format("{} '{}'", RamsesObjectTypeUtils::GetRamsesObjectTypeName(getType()), getName()) };
        StatusObjectImpl::addValidationMessage(EValidationSeverity::Info, std::move(message));

        return status;
    }

    status_t RamsesObjectImpl::addValidationMessage(EValidationSeverity severity, std::string message) const
    {
        message = fmt::format("{} '{}': {}", RamsesObjectTypeUtils::GetRamsesObjectTypeName(getType()), getName(), message);
        return StatusObjectImpl::addValidationMessage(severity, std::move(message));
    }
}
