//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NODETOPOLOGYTEST_H
#define RAMSES_NODETOPOLOGYTEST_H

#include "PerformanceTestBase.h"
#include "Collections/Vector.h"

namespace ramses
{
    class Node;
    class Scene;
}

class NodeTopologyTest : public PerformanceTestBase
{
public:

    enum
    {
        NodeTopologyTest_RemoveNodesIndividually = 0,
        NodeTopologyTest_RemoveNodesbyDestroyingParent
    };

    NodeTopologyTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void update() override;
    virtual void preUpdate() override;

private:

    void addChildren();
    void removeChildren();

    static const uint32_t NodeCount = 1000;

    ramses::Scene* m_scene;
    ramses::Node* m_parent;
    std::vector<ramses::Node*> m_children;
    std::vector<ramses::Node*> m_childrenShuffled;
};
#endif

