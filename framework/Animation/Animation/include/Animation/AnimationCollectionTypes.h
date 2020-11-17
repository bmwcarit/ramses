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
#include <vector>

namespace ramses_internal
{
    class AnimationDataBindBase;

    using SplineTimeStampVector = std::vector<SplineTimeStamp>;
    using DataBindHandleVector = std::vector<DataBindHandle>;
    using AnimationHandleVector = std::vector<AnimationHandle>;
    using ConstDataBindVector = std::vector<const AnimationDataBindBase *>;
}

#endif
