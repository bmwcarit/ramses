//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONCOMMON_H
#define RAMSES_ANIMATIONCOMMON_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Common/MemoryHandle.h"
#include "Common/TypedMemoryHandle.h"

namespace ramses_internal
{
    typedef UInt32 SplineTimeStamp;
    typedef UInt32 SplineKeyIndex;

    struct SplineHandleTag {};
    typedef TypedMemoryHandle<SplineHandleTag> SplineHandle;

    struct DataBindHandleTag{};
    typedef TypedMemoryHandle<DataBindHandleTag> DataBindHandle;

    struct AnimationInstanceHandleTag {};
    typedef TypedMemoryHandle<AnimationInstanceHandleTag> AnimationInstanceHandle;

    struct AnimationHandleTag {};
    typedef TypedMemoryHandle<AnimationHandleTag> AnimationHandle;

    const SplineTimeStamp         InvalidSplineTimeStamp         = SplineTimeStamp(-1);
    const SplineKeyIndex          InvalidSplineKeyIndex          = SplineKeyIndex(-1);

    enum ESplineKeyType
    {
        ESplineKeyType_Invalid = 0u,
        ESplineKeyType_Basic,
        ESplineKeyType_Tangents
    };

    enum EInterpolationType
    {
        EInterpolationType_Invalid = 0u,
        EInterpolationType_Step,
        EInterpolationType_Linear,
        EInterpolationType_Bezier
    };

    enum EVectorComponent
    {
        EVectorComponent_X = 0u,
        EVectorComponent_Y,
        EVectorComponent_Z,
        EVectorComponent_W,

        EVectorComponent_All
    };

    template <typename EDataType>
    class SplineKey;
    template <typename EDataType>
    class SplineKeyTangents;

    template <template<typename> class Key>
    struct SplineKeyToSplineKeyIDSelector
    {
    };

    template <>
    struct SplineKeyToSplineKeyIDSelector<SplineKey>
    {
        static const ESplineKeyType SplineKeyID = ESplineKeyType_Basic;
    };

    template <>
    struct SplineKeyToSplineKeyIDSelector < SplineKeyTangents >
    {
        static const ESplineKeyType SplineKeyID = ESplineKeyType_Tangents;
    };
}

#endif
