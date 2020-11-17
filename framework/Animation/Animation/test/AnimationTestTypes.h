//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_ANIMATIONTESTTYPES_H
#define RAMSES_ANIMATIONTESTTYPES_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class Vector3;
    template <typename EDataType>
    class SplineKeyTangents;
    template <template<typename> class Key, typename EDataType>
    class Spline;
    template <template<typename> class Key, typename EDataType>
    class SplineSolver;
    template <typename EDataType, UInt NumKeys>
    class SplineInitializer;
    template <typename EDataType, typename HandleType>
    class AnimationDataBindTestContainer;
    template < typename ContainerType, typename EDataType, typename HandleType, typename HandleType2>
    class AnimationDataBind;

    using SplineKeyVec3 = SplineKeyTangents<Vector3>;
    using SplineVec3 = Spline<SplineKeyTangents, Vector3>;
    using SplineSolverVec3 = SplineSolver<SplineKeyTangents, Vector3>;
    using SplineInitializerVec3_Small = SplineInitializer<Vector3, 10>;
    using SplineInitializerVec3_Medium = SplineInitializer<Vector3, 100>;
    using SplineInitializerVec3_Large = SplineInitializer<Vector3, 1000>;

    using ContainerType = AnimationDataBindTestContainer<Vector3>;
    using DataBindType = AnimationDataBind<ContainerType, Vector3, MemoryHandle>;
    using DataBindType2Handles = AnimationDataBind<ContainerType, Vector3, MemoryHandle, MemoryHandle>;
}

#endif
