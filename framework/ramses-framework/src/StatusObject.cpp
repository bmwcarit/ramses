//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-framework-api/StatusObject.h"

// internal
#include "StatusObjectImpl.h"
#include "Collections/HashSet.h"

namespace ramses
{
    StatusObject::StatusObject(std::unique_ptr<StatusObjectImpl> impl)
        : m_impl{ std::move(impl) }
    {
    }

    StatusObject::~StatusObject() = default;

    status_t StatusObject::validate() const
    {
        return m_impl->validate();
    }

    const char* StatusObject::getValidationReport(EValidationSeverity minSeverity) const
    {
        return m_impl->getValidationReport(minSeverity);
    }

    const char* StatusObject::getStatusMessage(status_t status) const
    {
        return m_impl->getStatusMessage(status);
    }
}
