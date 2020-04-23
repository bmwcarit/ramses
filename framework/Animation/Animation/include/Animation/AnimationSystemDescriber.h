//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMDESCRIBER_H
#define RAMSES_ANIMATIONSYSTEMDESCRIBER_H

#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/AnimationCommon.h"
#include "Animation/Spline.h"
#include "Animation/SplineKey.h"
#include "Animation/SplineKeyTangents.h"

namespace ramses_internal
{
    class IScene;
    class SplineBase;
    class SceneActionCollectionCreator;

    class AnimationSystemDescriber
    {
    public:
        static void DescribeAnimationSystem(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle);

    protected:
        static void CopySplines(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle);
        static void CopySpline(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget);
        static void CopySplineBasic(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget);
        static void CopySplineTangents(SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle, const SplineBase* const splineSource, SplineHandle splineTarget);

        static void CopyDataBinds(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle);
        static void CopyAnimationInstances(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle);
        static void CopyAnimations(const IAnimationSystem& animationSystemSource, SceneActionCollectionCreator& collector, AnimationSystemHandle animSystemHandle);

        template <typename EDataType>
        static EDataType GetSplineKeyValue(const SplineBase* const splineBase, SplineKeyIndex keyIndex);
        template <typename EDataType>
        static EDataType GetSplineKeyTangentsValue(const SplineBase* const splineBase, SplineKeyIndex keyIndex, Vector2& tanIn, Vector2& tanOut);
    };

    template <typename EDataType>
    EDataType AnimationSystemDescriber::GetSplineKeyValue(const SplineBase* const splineBase, SplineKeyIndex keyIndex)
    {
        const Spline<SplineKey, EDataType>& spline = static_cast<const Spline<SplineKey, EDataType>&>(*splineBase);
        const SplineKey<EDataType>& key = spline.getKey(keyIndex);
        return key.m_value;
    }

    template <typename EDataType>
    EDataType AnimationSystemDescriber::GetSplineKeyTangentsValue(const SplineBase* const splineBase, SplineKeyIndex keyIndex, Vector2& tanIn, Vector2& tanOut)
    {
        const Spline<SplineKeyTangents, EDataType>& spline = static_cast<const Spline<SplineKeyTangents, EDataType>&>(*splineBase);
        const SplineKeyTangents<EDataType>& key = spline.getKey(keyIndex);
        tanIn = key.m_tangentIn;
        tanOut = key.m_tangentOut;
        return key.m_value;
    }
}

#endif
