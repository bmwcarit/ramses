//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LogicNode.h"
#include "ramses-logic/Property.h"
#include <fmt/format.h>
#include <vector>
#include <deque>
#include <algorithm>

namespace rlogic
{
    class PropertyLinkTestUtils
    {
    public:
        // This util will collect all existing links in logic engine and match them with given expected links,
        // i.e. the expected links must not only exist but also be the only links in the engine.
        // This also tests all the relevant public API for querying links.
        static void ExpectLinks(const LogicEngine& logicEngine, std::vector<PropertyLink> expectedLinks)
        {
            // collect all exisintg links from logic engine
            const auto allObjects = logicEngine.getCollection<LogicObject>();

            std::vector<const Property*> allProperties;
            for (const auto obj : allObjects)
            {
                const auto logicNode = obj->as<LogicNode>();
                if (!logicNode)
                    continue;

                std::deque<const Property*> props{ logicNode->getInputs(), logicNode->getOutputs() };
                while (!props.empty())
                {
                    auto prop = props.back();
                    props.pop_back();
                    if (prop == nullptr)
                        continue;

                    allProperties.push_back(prop);

                    for (size_t i = 0u; i < prop->getChildCount(); ++i)
                        props.push_back(prop->getChild(i));
                }
            }

            std::vector<PropertyLink> allLinks;
            for (auto prop : allProperties)
            {
                if (prop->hasIncomingLink())
                    allLinks.push_back(*prop->getIncomingLink());
            }

            EXPECT_EQ(expectedLinks.size(), allLinks.size()) << "Count of expected links does not match actual count of existing links";
            if (expectedLinks.size() != allLinks.size())
                return;

            auto propertLinkComp = [](const PropertyLink& a, const PropertyLink& b) {
                assert(a.source != b.source || a.target != b.target);
                if (a.source != b.source)
                    return a.source < b.source;
                return a.target < b.target;
            };
            std::sort(expectedLinks.begin(), expectedLinks.end(), propertLinkComp);
            std::sort(allLinks.begin(), allLinks.end(), propertLinkComp);

            for (size_t i = 0u; i < allLinks.size(); ++i)
            {
                EXPECT_EQ(expectedLinks[i].source, allLinks[i].source) << fmt::format("expected and actual source of link #{} does not match", i);
                EXPECT_EQ(expectedLinks[i].target, allLinks[i].target) << fmt::format("expected and actual target of link #{} does not match", i);
                EXPECT_EQ(expectedLinks[i].isWeakLink, allLinks[i].isWeakLink) << fmt::format("expected and actual weak link flag of link #{} does not match", i);

                // checks for the other existing (mostly redundant) API
                EXPECT_TRUE(expectedLinks[i].source->isLinked());
                EXPECT_TRUE(expectedLinks[i].source->hasOutgoingLink());
                EXPECT_TRUE(expectedLinks[i].source->getOutgoingLinksCount() > 0u);
                EXPECT_TRUE(expectedLinks[i].target->isLinked());
                EXPECT_TRUE(expectedLinks[i].target->hasIncomingLink());
            }

            // test also logic engine getter
            auto engineLinks = logicEngine.getPropertyLinks();
            std::sort(engineLinks.begin(), engineLinks.end(), propertLinkComp);
            const bool equals = std::equal(engineLinks.cbegin(), engineLinks.cend(), expectedLinks.cbegin(), expectedLinks.cend(), [](const auto& l1, const auto& l2) {
                return l1.source == l2.source
                    && l1.target == l2.target
                    && l1.isWeakLink == l2.isWeakLink;
                });
            EXPECT_TRUE(equals) << "Links retrieved from LogicEngine API do not match!";
        }
    };
}
