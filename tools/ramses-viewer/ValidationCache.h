//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <unordered_map>
#include "ramses/framework/ValidationReport.h"
#include "ramses/client/SceneObject.h"
#include "impl/SceneObjectImpl.h"
#include <optional>

namespace ramses::internal
{
    class ValidationCache
    {
    public:
        explicit ValidationCache(const ramses::ValidationReport& sceneReport)
            : m_allIssues(sceneReport.getIssues())
        {
            // Sort the issues by owning object (required for getIssues())
            // (ramses-logic validation does not guarantee sorting)
            std::sort(m_allIssues.begin(), m_allIssues.end(), [](const Issue& a, const Issue& b) {
                const auto* objA = object_cast<const SceneObject*>(a.object);
                const auto* objB = object_cast<const SceneObject*>(b.object);
                const auto idA = objA ? objA->getSceneObjectId().getValue() : 0;
                const auto idB = objB ? objB->getSceneObjectId().getValue() : 0;
                return idA < idB;
            });
        }

        using iterator = std::vector<Issue>::const_iterator;

        /**
        * Gets the most severe issue type for this object or dependent objects
        */
        std::optional<EIssueType> getIssueType(const SceneObjectImpl& obj)
        {
            const auto found = m_cache.find(obj.getSceneObjectId());
            if (found != m_cache.end())
                return found->second;
            auto insert = m_cache.insert(found, {obj.getSceneObjectId(), calculateIssueType(obj)});
            assert(insert != m_cache.end());
            return insert->second;
        }

        /**
         * Gets all issues that are directly related to the given object
         */
        std::pair<iterator, iterator> getIssues(const SceneObjectImpl& obj) const
        {
            const auto& issues = m_allIssues;
            auto begin = std::find_if(issues.begin(), issues.end(), [&](const Issue& issue) { return issue.object == &obj.getRamsesObject(); });
            auto it = begin;
            for (; it != issues.end(); ++it)
            {
                if (it->object != &obj.getRamsesObject())
                    break;
            }
            return {begin, it};
        }

        const std::vector<Issue>& getAllIssues() const
        {
            return m_allIssues;
        }

    private:
        std::optional<EIssueType> calculateIssueType(const SceneObjectImpl& obj)
        {
            ramses::ValidationReport report;
            obj.validate(report.impl());
            if (report.hasError())
                return EIssueType::Error;
            if (report.hasIssue())
                return EIssueType::Warning;
            auto range = getIssues(obj);
            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->type == EIssueType::Error)
                    return EIssueType::Error;
            }
            if (range.first != range.second)
                return EIssueType::Warning;
            return std::nullopt;
        }

        std::vector<Issue> m_allIssues;
        std::unordered_map<sceneObjectId_t, std::optional<EIssueType>> m_cache;
    };
}
