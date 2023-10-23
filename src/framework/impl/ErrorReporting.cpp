//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/ErrorReporting.h"
#include "impl/RamsesObjectImpl.h"
#include "ramses/framework/RamsesObject.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    void ErrorReporting::set(std::string errorMessage, const RamsesObject* object)
    {
        if (object)
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "[{}] {}", object->getName(), errorMessage);
        }
        else
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "{}", errorMessage);
        }

        std::lock_guard<std::recursive_mutex> g{ m_lock };
        m_error = Issue{ ramses::EIssueType::Error, std::move(errorMessage), object };
    }

    void ErrorReporting::set(std::string errorMessage, const RamsesObjectImpl& object)
    {
        set(std::move(errorMessage), &object.getRamsesObject());
    }

    void ErrorReporting::reset()
    {
        std::lock_guard<std::recursive_mutex> g{ m_lock };
        m_error.reset();
    }

    std::optional<ramses::Issue> ErrorReporting::getError() const
    {
        std::lock_guard<std::recursive_mutex> g{ m_lock };
        return m_error;
    }
}
