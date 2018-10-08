//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StatusObjectImpl.h"
#include "Utils/LogMacros.h"
#include "Collections/StringOutputStream.h"
#include "Common/Cpp11Macros.h"
#include "PlatformAbstraction/PlatformGuard.h"

namespace ramses_internal
{
    static EValidationSeverityInternal convertToInternalEnum(ramses::EValidationSeverity severity)
    {
        switch (severity)
        {
        case ramses::EValidationSeverity_Info: return ramses_internal::EValidationSeverityInternal_Info;
        case ramses::EValidationSeverity_Warning: return ramses_internal::EValidationSeverityInternal_Warning;
        case ramses::EValidationSeverity_Error: return ramses_internal::EValidationSeverityInternal_Error;
        default: return EValidationSeverityInternal_Error;
        }
    }
}

namespace ramses
{
    StatusObjectImpl::StatusCache StatusObjectImpl::m_statusCache;
    ramses_internal::PlatformLightweightLock StatusObjectImpl::m_statusCacheLock;

    StatusObjectImpl::StatusObjectImpl()
        : m_hasErrorMessages(false)
    {
    }

    StatusObjectImpl::~StatusObjectImpl()
    {
    }

    status_t StatusObjectImpl::addStatusEntry(const char* message) const
    {
        ramses_internal::PlatformLightweightGuard g(m_statusCacheLock);
        return m_statusCache.addMessage(message);
    }

    status_t StatusObjectImpl::addErrorEntry(const char* message) const
    {
        LOG_ERROR(ramses_internal::CONTEXT_CLIENT, message);
        ramses_internal::PlatformLightweightGuard g(m_statusCacheLock);
        m_hasErrorMessages = true;
        return m_statusCache.addMessage(message);
    }

    const char* StatusObjectImpl::getStatusMessage(status_t status) const
    {
        ramses_internal::PlatformLightweightGuard g(m_statusCacheLock);
        return m_statusCache.getMessage(status);
    }

    status_t StatusObjectImpl::validate(uint32_t indent) const
    {
        status_t status = StatusOK;

        ramses_internal::PlatformLightweightGuard g(m_statusCacheLock);
        m_validationMessages.clear();
        if (m_hasErrorMessages)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "Following object has error status entries signaling wrong API usage, please check your error logs:");
            status = getValidationErrorStatusUnsafe();
            m_hasErrorMessages = false;
        }

        return status;
    }

    const char* StatusObjectImpl::getValidationReport(EValidationSeverity severityFilter) const
    {
        ramses_internal::StringOutputStream stringStream;
        ramses_foreach(m_validationMessages, message)
        {
            if (message->severity >= ramses_internal::convertToInternalEnum(severityFilter))
            {
                for (uint32_t i = 0u; i < message->indentation; ++i)
                {
                    stringStream << " ";
                }

                switch (message->severity)
                {
                default:
                case ramses_internal::EValidationSeverityInternal_Info:
                    break;
                case ramses_internal::EValidationSeverityInternal_Warning:
                    stringStream << "WARNING: ";
                    break;
                case ramses_internal::EValidationSeverityInternal_Error:
                    stringStream << "ERROR: ";
                    break;
                }

                stringStream << message->message;
                stringStream << "\n";
            }
        }

        m_validationReport = stringStream.c_str();
        return m_validationReport.c_str();
    }

    void StatusObjectImpl::addValidationObjectName(uint32_t indent, const ramses_internal::String& message) const
    {
        ValidationMessage validationMessage;
        validationMessage.severity = ramses_internal::EValidationSeverityInternal_ObjectName;
        validationMessage.indentation = indent;
        validationMessage.message = message;

        m_validationMessages.push_back(validationMessage);
    }

    void StatusObjectImpl::addValidationMessage(EValidationSeverity severity, uint32_t indent, const ramses_internal::String& message) const
    {
        ValidationMessage validationMessage;
        validationMessage.severity = ramses_internal::convertToInternalEnum(severity);
        validationMessage.indentation = indent;
        validationMessage.message = message;

        m_validationMessages.push_back(validationMessage);
    }

    status_t StatusObjectImpl::addValidationOfDependentObject(uint32_t indent, const StatusObjectImpl& dependentObject) const
    {
        const status_t status = dependentObject.validate(indent);

        if (status != StatusOK)
        {
            ramses_foreach(dependentObject.m_validationMessages, message)
            {
                m_validationMessages.push_back(*message);
            }
        }

        return status;
    }

    status_t StatusObjectImpl::getValidationErrorStatus() const
    {
        ramses_internal::PlatformLightweightGuard g(m_statusCacheLock);
        return getValidationErrorStatusUnsafe();
    }

    status_t StatusObjectImpl::getValidationErrorStatusUnsafe() const
    {
        return m_statusCache.addMessage("Validation of object revealed issues, retrieve validation report by calling getValidationReport() on affected object");
    }
}
