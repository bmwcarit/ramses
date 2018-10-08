//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONCOLLECTIONTYPES_H
#define RAMSES_ANIMATIONCOLLECTIONTYPES_H

#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    template<typename T>
    class Vector;
    class AnimationDataBindBase;

    typedef Vector<SplineTimeStamp> SplineTimeStampVector;
    typedef Vector<DataBindHandle> DataBindHandleVector;
    typedef Vector<AnimationHandle> AnimationHandleVector;
    typedef Vector<const AnimationDataBindBase*> ConstDataBindVector;
}

#endif
