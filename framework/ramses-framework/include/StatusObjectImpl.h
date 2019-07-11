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

// framework
#include "Utils/MessagePool.h"
#include "Collections/String.h"
#include "PlatformAbstraction/PlatformLock.h"

//logging utils
#include "APILoggingMacros.h"
#include "ramses-framework-api/EValidationSeverity.h"

namespace ramses_internal
{
    enum EValidationSeverityInternal
    {
        EValidationSeverityInternal_Info= 0,
        EValidationSeverityInternal_Warning,
        EValidationSeverityInternal_Error,
        EValidationSeverityInternal_ObjectName
    };
}

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
        StatusObjectImpl();
        virtual ~StatusObjectImpl();

        status_t         addErrorEntry(const char* message) const;
        status_t         addStatusEntry(const char* message) const;
        const char*      getStatusMessage(status_t status) const;

        virtual status_t validate(uint32_t indent) const;
        const char*      getValidationReport(EValidationSeverity severity) const;

    protected:
        void             addValidationMessage(EValidationSeverity severity, uint32_t indent, const ramses_internal::String& message) const;
        void             addValidationObjectName(uint32_t indent, const ramses_internal::String& message) const;
        status_t         addValidationOfDependentObject(uint32_t indent, const StatusObjectImpl& dependentObject) const;

        status_t         getValidationErrorStatus() const;

        static const uint32_t IndentationStep = 4u;

    private:
        status_t         getValidationErrorStatusUnsafe() const;

        struct ValidationMessage
        {
            ramses_internal::EValidationSeverityInternal severity;
            uint32_t                indentation;
            ramses_internal::String message;
        };
        typedef std::vector<ValidationMessage> ValidationMessages;

        mutable bool                    m_hasErrorMessages;
        mutable ValidationMessages      m_validationMessages;
        mutable ramses_internal::String m_validationReport;

        static ramses_internal::PlatformLightweightLock m_statusCacheLock;
        typedef ramses_internal::MessagePool<32u, StatusOK> StatusCache;
        static StatusCache m_statusCache;
    };
}

#endif
