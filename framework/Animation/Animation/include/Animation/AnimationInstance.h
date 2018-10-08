//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONINSTANCE_H
#define RAMSES_ANIMATIONINSTANCE_H

#include "Collections/Vector.h"
#include "Animation/AnimationCommon.h"
#include "Animation/AnimationCollectionTypes.h"

namespace ramses_internal
{
    class AnimationInstance
    {
    public:
        AnimationInstance(SplineHandle splineHandle = SplineHandle::Invalid(), EInterpolationType interpolationType = EInterpolationType_Invalid, EVectorComponent vectorComponent = EVectorComponent_All);

        SplineHandle getSplineHandle() const;
        EInterpolationType getInterpolationType() const;
        EVectorComponent getVectorComponentFlag() const;
        void addDataBinding(DataBindHandle handle);
        Bool hasDataBinding(DataBindHandle handle) const;
        const DataBindHandleVector& getDataBindings() const;

        Bool operator==(const AnimationInstance& other) const;
        Bool operator!=(const AnimationInstance& other) const;

        static const AnimationInstance& InvalidInstance();

    private:
        SplineHandle m_splineHandle;
        EInterpolationType m_interpolationType;
        EVectorComponent m_vectorComponent;
        DataBindHandleVector m_dataBindHandles;
    };

    inline SplineHandle AnimationInstance::getSplineHandle() const
    {
        return m_splineHandle;
    }

    inline EInterpolationType AnimationInstance::getInterpolationType() const
    {
        return m_interpolationType;
    }

    inline EVectorComponent AnimationInstance::getVectorComponentFlag() const
    {
        return m_vectorComponent;
    }

    inline const DataBindHandleVector& AnimationInstance::getDataBindings() const
    {
        return m_dataBindHandles;
    }

    inline Bool AnimationInstance::operator==(const AnimationInstance& other) const
    {
        return m_splineHandle == other.m_splineHandle
            && m_interpolationType == other.m_interpolationType
            && m_vectorComponent == other.m_vectorComponent
            && m_dataBindHandles == m_dataBindHandles;
    }

    inline Bool AnimationInstance::operator!=(const AnimationInstance& other) const
    {
        return !operator==(other);
    }
}

#endif
