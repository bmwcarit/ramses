//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/ValidationResults.h"
#include "ramses-logic/LogicNode.h"
#include "impl/LoggerImpl.h"
#include "impl/LogicObjectImpl.h"

namespace rlogic::internal
{
    void ValidationResults::add(std::string warningMessage, const LogicObject* logicObject, EWarningType type)
    {
        if (logicObject)
        {
            LOG_WARN("[{}] {}", logicObject->m_impl->getIdentificationString(), warningMessage);
        }
        else
        {
            LOG_WARN("{}", warningMessage);
        }

        m_warnings.emplace_back(WarningData{ std::move(warningMessage), type, logicObject });
    }

    void ValidationResults::clear()
    {
        m_warnings.clear();
    }

    const std::vector<WarningData>& ValidationResults::getWarnings() const
    {
        return m_warnings;
    }
}
