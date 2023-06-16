//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STATUSOBJECTIMPL_H
#define RAMSES_STATUSOBJECTIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/EValidationSeverity.h"
#include "Utils/MessagePool.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "APILoggingMacros.h"

#include <unordered_set>
#include <string>

#define CHECK_RETURN_ERR(expr) \
    { \
        const ramses::status_t statusRet = (expr); \
        if (statusRet != ramses::StatusOK) \
        { \
            return statusRet; \
        } \
    }

namespace ramses
{
    class StatusObjectImpl
    {
    public:
        StatusObjectImpl() = default;
        virtual ~StatusObjectImpl() = default;

        RNODISCARD status_t addErrorEntry(const char* message) const;
        RNODISCARD status_t addErrorEntry(const std::string& message) const;
        const char*         getStatusMessage(status_t status) const;

        virtual status_t    validate() const;
        const char*         getValidationReport(EValidationSeverity minSeverity) const;

    protected:
        virtual status_t    addValidationMessage(EValidationSeverity severity, std::string message) const;
        status_t            addValidationOfDependentObject(const StatusObjectImpl& dependentObject) const;

    private:
        EValidationSeverity getMaxValidationSeverity() const;
        void writeMessagesToStream(EValidationSeverity minSeverity, ramses_internal::StringOutputStream& stream, size_t indent, std::unordered_set<const StatusObjectImpl*>& visitedObjs) const;

        struct ValidationMessage
        {
            EValidationSeverity severity;
            std::string message;
        };
        using ValidationMessages = std::vector<ValidationMessage>;

        mutable ValidationMessages      m_validationMessages;
        mutable std::string m_validationReport;
        mutable std::vector<const StatusObjectImpl*> m_dependentObjects;

        static std::recursive_mutex m_statusCacheLock;
        using StatusCache = ramses_internal::MessagePool<32U, StatusOK>;
        static StatusCache m_statusCache;
    };
}

#endif
