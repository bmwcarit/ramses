//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONPROCESSDATA_H
#define RAMSES_ANIMATIONPROCESSDATA_H

#include "Animation/Animation.h"
#include "Animation/SplineIterator.h"
#include "Animation/AnimationCollectionTypes.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    class SplineBase;
    class AnimationDataBindBase;

    struct AnimationProcessData
    {
        Animation m_animation;
        SplineIterator m_splineIterator;
        const SplineBase* m_spline;
        ConstDataBindVector m_dataBinds;
        EInterpolationType m_interpolationType;
        EVectorComponent m_dataComponent;
    };

    struct AnimationSplineData
    {
        SplineBase* m_spline;
        SplineTimeStamp m_timeStampToSet;
        SplineTimeStamp m_timeStampCurrent;
        EInterpolationType m_interpolationType;
    };
}

#endif

