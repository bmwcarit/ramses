//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimatedPropertyUtils.h"

namespace ramses
{
    bool AnimatedPropertyUtils::isComponentMatchingTransformNode(EAnimatedPropertyComponent ePropertyComponent)
    {
        return isComponentValidVec3(ePropertyComponent);
    }

    bool AnimatedPropertyUtils::isComponentMatchingInputFloat(EAnimatedPropertyComponent ePropertyComponent)
    {
        return isComponentValidScalar(ePropertyComponent);
    }

    bool AnimatedPropertyUtils::isComponentMatchingInputVec4(EAnimatedPropertyComponent ePropertyComponent)
    {
        return isComponentValidVec4(ePropertyComponent);
    }

    bool AnimatedPropertyUtils::isComponentMatchingEffectInput(EAnimatedPropertyComponent ePropertyComponent, ramses_internal::EDataType dataType)
    {
        const ramses_internal::UInt32 numberOfComponents = EnumToNumComponents(dataType);
        switch (ePropertyComponent)
        {
        case EAnimatedPropertyComponent_All:
        case EAnimatedPropertyComponent_X:
            return true;
        case EAnimatedPropertyComponent_Y:
            return (numberOfComponents >= 2);
        case EAnimatedPropertyComponent_Z:
            return (numberOfComponents >= 3);
        case EAnimatedPropertyComponent_W:
            return (numberOfComponents >= 4);
        default:
            return false;
        }
    }

    bool AnimatedPropertyUtils::isComponentValidScalar(EAnimatedPropertyComponent ePropertyComponent)
    {
        return (ePropertyComponent == EAnimatedPropertyComponent_All) || (ePropertyComponent == EAnimatedPropertyComponent_X);
    }

    bool AnimatedPropertyUtils::isComponentValidVec3(EAnimatedPropertyComponent ePropertyComponent)
    {
        return (ePropertyComponent == EAnimatedPropertyComponent_All) || (ePropertyComponent <= EAnimatedPropertyComponent_Z);
    }

    bool AnimatedPropertyUtils::isComponentValidVec4(EAnimatedPropertyComponent ePropertyComponent)
    {
        UNUSED(ePropertyComponent);
        return true;
    }

    ramses_internal::EVectorComponent AnimatedPropertyUtils::getVectorComponentFromProperty(EAnimatedPropertyComponent ePropertyComponent)
    {
        switch (ePropertyComponent)
        {
        case EAnimatedPropertyComponent_X:
            return ramses_internal::EVectorComponent_X;
        case EAnimatedPropertyComponent_Y:
            return ramses_internal::EVectorComponent_Y;
        case EAnimatedPropertyComponent_Z:
            return ramses_internal::EVectorComponent_Z;
        case EAnimatedPropertyComponent_W:
            return ramses_internal::EVectorComponent_W;
        case EAnimatedPropertyComponent_All:
        default:
            return ramses_internal::EVectorComponent_All;
        }
    }
}
