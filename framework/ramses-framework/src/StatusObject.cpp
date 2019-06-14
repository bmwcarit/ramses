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

namespace ramses
{
    StatusObject::StatusObject(StatusObjectImpl& pimpl)
        : impl(pimpl)
    {
    }

    StatusObject::~StatusObject()
    {
        delete &impl;
    }

    status_t StatusObject::validate() const
    {
        return impl.validate(0u);
    }

    const char* StatusObject::getValidationReport(EValidationSeverity severity) const
    {
        return impl.getValidationReport(severity);
    }

    const char* StatusObject::getStatusMessage(status_t status) const
    {
        return impl.getStatusMessage(status);
    }

}
