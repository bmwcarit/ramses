//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BOUNDINGSPHERETEST_H
#define RAMSES_BOUNDINGSPHERETEST_H

#include "PerformanceTestBase.h"

namespace ramses
{
    class BoundingSphereCollection;
}

class BoundingSphereTest : public PerformanceTestBase
{
public:

    enum
    {
        BoundingSphereTest_1K_BinaryTree = 0,
        BoundingSphereTest_4K_BinaryTree,
        BoundingSphereTest_16K_BinaryTree,

        BoundingSphereTest_1K_SpreadOutTree,
        BoundingSphereTest_4K_SpreadOutTree,
        BoundingSphereTest_16K_SpreadOutTree,
    };

    BoundingSphereTest(ramses_internal::String testName, uint32_t testState);
    virtual ~BoundingSphereTest();

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void update() override;

private:

    ramses::Node* m_rootNode;
    ramses::BoundingSphereCollection* m_spheres;
};
#endif

