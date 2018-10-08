//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SPLINEITERATORTEST_H
#define RAMSES_SPLINEITERATORTEST_H

#include "framework_common_gmock_header.h"
#include "SplineTestUtils.h"
#include "AnimationTestTypes.h"

namespace ramses_internal
{
    class ASplineIterator : public testing::Test
    {
    public:
        ASplineIterator()
        {
            m_splineInit.initValuesRandom();
            m_splineInit.initSplineWithValues();
        }

    protected:
        SplineInitializerVec3_Small m_splineInit;

        void stateBeforeSplineStart();
        void stateAfterSplineStart();
        void stateAtSplineKeys();
        void stateInBetweenSplineKeys();
    };
}

#endif
