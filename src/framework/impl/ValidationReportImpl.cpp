//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// impl
#include "impl/ValidationReportImpl.h"
// internal
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "ramses/framework/RamsesObject.h"
#include "impl/RamsesObjectImpl.h"
#include "impl/RamsesObjectTypeUtils.h"

namespace ramses::internal
{
    std::string ValidationReportImpl::toString() const
    {
        StringOutputStream stringStream;
        for (auto& m: m_messages)
        {
            switch (m.type)
            {
            case EIssueType::Error:
                stringStream << "ERROR: ";
                break;
            case EIssueType::Warning:
                stringStream << "WARNING: ";
                break;

            }

            if (m.object)
                stringStream << m.object->impl().getIdentificationString() << ": ";

            stringStream << m.message << "\n";
        }
        return stringStream.release();
    }

    const std::vector<const RamsesObjectImpl*>& ValidationReportImpl::getDependentObjects(const RamsesObjectImpl* object) const
    {
        auto it = m_dependencies.find(object);
        if (it != m_dependencies.end())
        {
            return it->second;
        }
        static std::vector<const RamsesObjectImpl*> emptyResult;
        return emptyResult;
    }
}
