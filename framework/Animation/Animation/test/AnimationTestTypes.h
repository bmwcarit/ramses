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

    typedef SplineKeyTangents<Vector3> SplineKeyVec3;
    typedef Spline<SplineKeyTangents, Vector3> SplineVec3;
    typedef SplineSolver<SplineKeyTangents, Vector3> SplineSolverVec3;
    typedef SplineInitializer<Vector3, 10> SplineInitializerVec3_Small;
    typedef SplineInitializer<Vector3, 100> SplineInitializerVec3_Medium;
    typedef SplineInitializer<Vector3, 1000> SplineInitializerVec3_Large;

    typedef AnimationDataBindTestContainer<Vector3> ContainerType;
    typedef AnimationDataBind<ContainerType, Vector3, MemoryHandle> DataBindType;
    typedef AnimationDataBind<ContainerType, Vector3, MemoryHandle, MemoryHandle> DataBindType2Handles;
}

#endif
