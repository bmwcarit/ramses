//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/APIExport.h"
#include "ramses/framework/Issue.h"
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace ramses::internal
{
    class RamsesObjectImpl;

    class ValidationReportImpl
    {
    public:
        void add(EIssueType type, const std::string& text, const RamsesObject* obj)
        {
            m_messages.push_back({type, text, obj});
            m_resultingType = std::min(m_resultingType, type);
        }

        void addDependentObject(const RamsesObjectImpl& object, const RamsesObjectImpl& dependency)
        {
            m_dependencies[&object].push_back(&dependency);
        }

        [[nodiscard]] const std::vector<const RamsesObjectImpl*>& getDependentObjects(const RamsesObjectImpl* object) const;

        void clear()
        {
            m_messages.clear();
            m_resultingType = EIssueType::Warning;
            m_dependencies.clear();
            m_visited.clear();
        }

        [[nodiscard]] bool hasError() const
        {
            return !m_messages.empty() && (m_resultingType == EIssueType::Error);
        }

        [[nodiscard]] bool hasIssue() const
        {
            return !m_messages.empty();
        }

        [[nodiscard]] const std::vector<Issue>& getIssues() const
        {
            return m_messages;
        }

        [[nodiscard]] bool addVisit(const RamsesObjectImpl* obj)
        {
            const auto res = m_visited.insert(obj);
            return res.second;
        }

        [[nodiscard]] std::string toString() const;

    private:
        using DependencyMap = std::unordered_map<const RamsesObjectImpl*, std::vector<const RamsesObjectImpl*>>;

        std::vector<Issue> m_messages;
        EIssueType         m_resultingType = EIssueType::Warning;
        DependencyMap      m_dependencies;

        std::unordered_set<const RamsesObjectImpl*> m_visited;
    };
}
