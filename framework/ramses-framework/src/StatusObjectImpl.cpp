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

#include <cassert>

namespace ramses
{
    StatusObjectImpl::StatusCache StatusObjectImpl::m_statusCache;
    std::recursive_mutex StatusObjectImpl::m_statusCacheLock;

    status_t StatusObjectImpl::addErrorEntry(const char* message) const
    {
        LOG_ERROR(ramses_internal::CONTEXT_CLIENT, message);
        std::lock_guard<std::recursive_mutex> g(m_statusCacheLock);
        return m_statusCache.addMessage(message);
    }

    RNODISCARD status_t StatusObjectImpl::addErrorEntry(const std::string& message) const
    {
        return addErrorEntry(message.c_str());
    }

    const char* StatusObjectImpl::getStatusMessage(status_t status) const
    {
        std::lock_guard<std::recursive_mutex> g(m_statusCacheLock);
        return m_statusCache.getMessage(status);
    }

    status_t StatusObjectImpl::validate() const
    {
        status_t status = StatusOK;

        m_dependentObjects.clear();

        std::lock_guard<std::recursive_mutex> g(m_statusCacheLock);
        m_validationMessages.clear();

        return status;
    }

    const char* StatusObjectImpl::getValidationReport(EValidationSeverity minSeverity) const
    {
        ramses_internal::StringOutputStream stringStream;
        std::unordered_set<const StatusObjectImpl*> visitedObjs;
        writeMessagesToStream(minSeverity, stringStream, 0u, visitedObjs);
        m_validationReport = stringStream.c_str();

        return m_validationReport.c_str();
    }

    status_t StatusObjectImpl::addValidationMessage(EValidationSeverity severity, std::string message) const
    {
        m_validationMessages.push_back({ severity, std::move(message) });
        std::lock_guard<std::recursive_mutex> g(m_statusCacheLock);
        switch (severity)
        {
        default:
        case ramses::EValidationSeverity::Info:
            return StatusOK;
        case ramses::EValidationSeverity::Warning:
            return m_statusCache.addMessage("Validation warning");
        case ramses::EValidationSeverity::Error:
            return m_statusCache.addMessage("Validation error");
        }
    }

    status_t StatusObjectImpl::addValidationOfDependentObject(const StatusObjectImpl& dependentObject) const
    {
        assert(&dependentObject != this);
        m_dependentObjects.push_back(&dependentObject);

        return dependentObject.validate();
    }

    EValidationSeverity StatusObjectImpl::getMaxValidationSeverity() const
    {
        EValidationSeverity maxSeverity = EValidationSeverity::Info;
        for (const auto& msg : m_validationMessages)
            maxSeverity = std::max(maxSeverity, msg.severity);

        return maxSeverity;
    }

    void StatusObjectImpl::writeMessagesToStream(EValidationSeverity minSeverity, ramses_internal::StringOutputStream& stream, size_t indent, std::unordered_set<const StatusObjectImpl*>& visitedObjs) const
    {
        if (minSeverity > EValidationSeverity::Info && visitedObjs.count(this) != 0)
            return;
        visitedObjs.insert(this);

        std::string indentStr;
        if (minSeverity == EValidationSeverity::Info)
            indentStr.assign(indent * 2u, ' ');

        // write this object's messages
        for (const auto& message : m_validationMessages)
        {
            if (message.severity >= minSeverity)
            {
                stream << indentStr;
                switch (message.severity)
                {
                default:
                case EValidationSeverity::Info:
                    break;
                case EValidationSeverity::Warning:
                    stream << "WARNING: ";
                    break;
                case EValidationSeverity::Error:
                    stream << "ERROR: ";
                    break;
                }
                stream << message.message << "\n";
            }
        }

        // write all dependent objects' messages (recursively)
        if (!m_dependentObjects.empty())
        {
            if (minSeverity == EValidationSeverity::Info)
                stream << indentStr << "- " << m_dependentObjects.size() << " dependent objects:\n";
            for (const auto& dep : m_dependentObjects)
                dep->writeMessagesToStream(minSeverity, stream, indent + 1, visitedObjs);
        }
    }
}
