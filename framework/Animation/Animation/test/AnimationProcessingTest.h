//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSINGTEST_H
#define RAMSES_ANIMATIONPROCESSINGTEST_H

#include "Animation/AnimationLogic.h"
#include "Animation/AnimationProcessing.h"
#include "AnimationTestUtils.h"

namespace ramses_internal
{
    class AnimationProcessingTest : public AnimationTest
    {
    public:
        AnimationProcessingTest()
            : AnimationTest()
            , m_lastSplineKeyIndex(InvalidSplineKeyIndex)
            , m_logic(m_animationData)
            , m_processing(m_animationData)
        {
        }

        virtual void init();

    protected:
        virtual SplineHandle initSpline(AnimationData& resMgr);

        SplineVec3 m_spline;
        SplineKeyIndex m_lastSplineKeyIndex;
        SplineHandle m_splineHandle;
        AnimationHandle m_animationHandle;

        AnimationLogic m_logic;
        AnimationProcessing m_processing;

        void animatedDataAtStartKey(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);
        void animatedDataAtEndKey(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);
        void animatedDataAtKeys(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);
        void animatedDataInBetweenKeys(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);
        void animatedDataIncrementalTimeAdvance(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);
        void animatedDataIncrementalTimeAdvanceWithTargetChanges(const Vector3& initVal1 = Vector3(0), const Vector3& initVal2 = Vector3(0), Animation::Flags flags = 0u);

        template <typename VectorData>
        static void AnimateSingleComponentDataTest(EVectorComponent component, EVectorComponent maxTypeComponent);
    };
}

#endif
