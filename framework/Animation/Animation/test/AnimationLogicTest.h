//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONLOGICTEST_H
#define RAMSES_ANIMATIONLOGICTEST_H

#include "AnimationTestUtils.h"
#include "Animation/AnimationLogic.h"

namespace ramses_internal
{
    class AnimationLogicTest : public AnimationTest
    {
    public:
        AnimationLogicTest()
            : AnimationTest()
            , m_logic(m_animationData)
        {
        }

        virtual void init() override
        {
            AnimationTest::init();
            m_logic.addListener(&m_stateListener);
        }

    protected:
        AnimationLogic m_logic;
    };
}

#endif
