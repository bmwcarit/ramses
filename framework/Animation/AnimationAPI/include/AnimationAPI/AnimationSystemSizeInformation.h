//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMSIZEINFORMATION_H
#define RAMSES_ANIMATIONSYSTEMSIZEINFORMATION_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    struct AnimationSystemSizeInformation
    {
        AnimationSystemSizeInformation(
            UInt32 splines = 0,
            UInt32 dataBinds = 0,
            UInt32 animationInstances = 0,
            UInt32 animations = 0)
            : splineCount(splines)
            , dataBindCount(dataBinds)
            , animationInstanceCount(animationInstances)
            , animationCount(animations)
        {
        }

        bool operator==(const AnimationSystemSizeInformation& other) const
        {
            return (splineCount == other.splineCount)
                && (dataBindCount == other.dataBindCount)
                && (animationInstanceCount == other.animationInstanceCount)
                && (animationCount == other.animationCount);
        }

        UInt32 splineCount;
        UInt32 dataBindCount;
        UInt32 animationInstanceCount;
        UInt32 animationCount;
    };
}

#endif
