//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/AnimationInstance.h"

namespace ramses_internal
{
    AnimationInstance::AnimationInstance(SplineHandle splineHandle, EInterpolationType interpolationType, EVectorComponent vectorComponent)
        : m_splineHandle(splineHandle)
        , m_interpolationType(interpolationType)
        , m_vectorComponent(vectorComponent)
    {
    }

    void AnimationInstance::addDataBinding(DataBindHandle handle)
    {
        if (!hasDataBinding(handle))
        {
            m_dataBindHandles.push_back(handle);
        }
    }

    Bool AnimationInstance::hasDataBinding(DataBindHandle handle) const
    {
        return contains_c(m_dataBindHandles, handle);
    }

    const AnimationInstance& AnimationInstance::InvalidInstance()
    {
        static AnimationInstance invalidInstance;
        return invalidInstance;
    }
}
