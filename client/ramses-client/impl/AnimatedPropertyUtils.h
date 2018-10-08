//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDPROPERTYUTILS_H
#define RAMSES_ANIMATEDPROPERTYUTILS_H

#include "ramses-client-api/AnimatedProperty.h"
#include "Animation/AnimationCommon.h"
#include "SceneAPI/EDataType.h"

namespace ramses
{
    class AnimatedPropertyUtils
    {
    public:
        static bool isComponentMatchingTransformNode(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentMatchingMeshNode(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentMatchingInputFloat(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentMatchingInputVec4(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentMatchingEffectInput(EAnimatedPropertyComponent ePropertyComponent, ramses_internal::EDataType dataType);
        static ramses_internal::EVectorComponent getVectorComponentFromProperty(EAnimatedPropertyComponent ePropertyComponent);

    private:
        AnimatedPropertyUtils() {}
        ~AnimatedPropertyUtils() {}

        static bool isComponentValidScalar(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentValidVec3(EAnimatedPropertyComponent ePropertyComponent);
        static bool isComponentValidVec4(EAnimatedPropertyComponent ePropertyComponent);

    };
}

#endif
