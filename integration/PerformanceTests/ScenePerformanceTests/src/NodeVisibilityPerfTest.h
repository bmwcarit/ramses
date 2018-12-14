//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NODEVISIBILITYPERFTEST_H
#define RAMSES_NODEVISIBILITYPERFTEST_H

#include "PerformanceTestBase.h"

namespace ramses
{
    class Node;
    class Scene;
}

class NodeVisibilityPerfTest : public PerformanceTestBase
{
public:

    enum
    {
        NodeVisibilityTest_DeepGraph_RootNode = 0,
        NodeVisibilityTest_DeepGraph_MiddleNode,
        NodeVisibilityTest_DeepGraph_LeafNode,
        NodeVisibilityTest_WideGraph_RootNode,
    };

    NodeVisibilityPerfTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void preUpdate() override;
    virtual void update() override;

private:

    ramses::Node* injectVisibilityNodeAsParent(ramses::Node* target);

    ramses::Scene* m_scene;
    ramses::Node* m_targetNode;
    bool m_isVisible;
};
#endif

