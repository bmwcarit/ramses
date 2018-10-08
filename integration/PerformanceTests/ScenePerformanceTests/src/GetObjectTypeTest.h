//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_GETOBJECTTYPETEST_H
#define RAMSES_GETOBJECTTYPETEST_H

#include "PerformanceTestBase.h"

namespace ramses
{
    class TransformationNode;
}

class GetObjectTypeTest : public PerformanceTestBase
{
public:

    enum
    {
        GetObjectTypeTest_GetType = 0,
        GetObjectTypeTest_IsOfType,
    };

    GetObjectTypeTest(ramses_internal::String testName, uint32_t testState);

    virtual void initTest(ramses::RamsesClient& client, ramses::Scene& scene) override;
    virtual void preUpdate() override;
    virtual void update() override;

private:
    ramses::TransformationNode* m_node;

};
#endif

